#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static pos targetPos;
static creature *monst;
static enum monsterTypes monstType;
static int healthBefore;
static int monsterCountBefore;
static item *theItem;
static creature *sweepTargets[3];


// --- shared setup helpers ---

static creature *placeVisibleMonster(enum monsterTypes type, pos loc) {
    creature *monst = spawnMonster(type, loc);
    revealCell(loc);
    return monst;
}

static void protectEquipmentFromAcid(void) {
    theItem = equippedWeapon();
    theItem->flags |= ITEM_PROTECTED;

    theItem = equippedArmor();
    theItem->flags |= ITEM_PROTECTED;
}

static void target_setup(enum monsterTypes typ) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monstType = typ;
    monst = placeHostileMonster(monstType, targetPos);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean attack_once_step(void) {
    moveRight();
    return false;
}

static boolean monster_attacks_step(void) {
    // Have the monster attack the player directly.
    monsterAttacksPlayer(monst);
    return false;
}

static boolean target_was_wounded(void) {
    TEST_ASSERT(monsterIsHurt(monst), "monster should be dead or wounded");
    return !testFailed;
}

// ============================================================
// BR65933 — an attack always hits a paralyzed defender
// ============================================================

static void paralyzed_target_setup(void) {
    // TODO: Test different paralyzable, non-invulnerable monsters
    target_setup(MK_TROLL);
}


// ============================================================
// BR66846 — a sleeping defender is caught flat-footed and always hit
// ============================================================

static void sleeping_target_setup(void) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monst = placeVisibleMonster(MK_TROLL, targetPos);
    setMonsterHealth(monst, monst->info.maxHP);
    putMonsterToSleep(monst);
}

static boolean sleeping_target_was_wounded(void) {
    TEST_ASSERT(monsterIsHurt(monst),
                "a sleeping defender should be hit");
    return !testFailed;
}

// ============================================================
// BR89965 — a struck sleeping monster wakes and tracks the player
// ============================================================

static boolean struck_monster_woke(void) {
    TEST_ASSERT(!monsterIsAsleep(monst), "the struck monster should wake");
    TEST_ASSERT(monsterIsTrackingPlayer(monst),
                "the woken monster should begin tracking the player");
    return !testFailed;
}

// ============================================================
// BR62243 — attacking a wandering monster alerts it
// ============================================================

static void unaware_target_setup(void) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);
    protectEquipmentFromAcid();

    enum monsterBehaviorFlags excludeBehaviors = MONST_INANIMATE | MONST_IMMOBILE | MONST_ALWAYS_HUNTING | MONST_NEVER_SLEEPS | MONST_GETS_TURN_ON_ACTIVATION | MONST_RESTRICTED_TO_LIQUID;
    enum monsterAbilityFlags excludeAbilities = MA_KAMIKAZE;
    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, excludeBehaviors, 0, excludeAbilities);
    monst = placeVisibleMonster(monstType, targetPos);
    makeMonsterUnaware(monst);
}

static boolean monster_was_alerted(void) {
    TEST_ASSERT(!monsterIsWandering(monst), "the monster should no longer be unaware");
    TEST_ASSERT(monsterIsTrackingPlayer(monst),
                "the alerted monster should track the player");
    return !testFailed;
}

// ============================================================
// BR33297 — a sweeping weapon strikes every adjacent enemy
// ============================================================

static void sweep_setup(void) {
    clearArena();
    theItem = givePlayerItem(WEAPON, AXE, 20);
    equip(theItem);
    drink(POTION_STRENGTH); // STR 13
    drink(POTION_STRENGTH); // STR 14
    drink(POTION_STRENGTH); // STR 15 (axe)

    static const int offsets[3][2] = { {1, 0}, {0, -1}, {0, 1} };
    for (int i = 0; i < 3; i++) {
        pos loc = posOffset(getPlayerPos(), offsets[i][0], offsets[i][1]);
        sweepTargets[i] = placeHostileMonster(MK_KRAKEN, loc); // Kraken is helpless
        putMonsterToSleep(sweepTargets[i]); // Guaranteed hit
    }
}

static boolean all_adjacent_were_hit(void) {
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(monsterIsHurt(sweepTargets[i]), "the sweeping attack should hit every adjacent enemy");
    }
    return !testFailed;
}

// ============================================================
// BR77279 — a creature immune to weapons takes no melee damage
// ============================================================

static void weapon_immune_setup(void) {
    clearArena();
    monst = placeHostileMonster(MK_REVENANT, posOffset(getPlayerPos(), 1, 0));
    applyStatusToMonster(monst, STATUS_PARALYZED, 20); // Guaranteed hit
    healthBefore = monsterHealth(monst);
}

static boolean immune_target_unharmed(void) {
    TEST_ASSERT(monsterIsHealthy(monst), "a weapon-immune creature should take no melee damage");
    return !testFailed;
}

// ============================================================
// BR81065 — an invulnerable creature cannot be damaged by an attack
// ============================================================

static void invulnerable_setup(void) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monst = placeHostileMonster(MK_WARDEN_OF_YENDOR, targetPos);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean invulnerable_target_unharmed(void) {
    TEST_ASSERT(monsterIsHealthy(monst), "an invulnerable creature should take no damage");
    return !testFailed;
}

// ============================================================
// BR76528 — a blow that reduces a creature to zero health kills it
// ============================================================

static void lethal_blow_setup(void) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);
    protectEquipmentFromAcid();

    enum monsterBehaviorFlags excludeBehaviors = MONST_INVULNERABLE | MONST_IMMUNE_TO_WEAPONS;
    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, excludeBehaviors, 0, 0);
    monst = placeHostileMonster(monstType, targetPos);
    setMonsterHealth(monst, 1);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean monster_was_killed(void) {
    TEST_ASSERT(monsterIsDead(monst), "the monster should have been killed");
    return !testFailed;
}

// ============================================================
// BR31952 — a slain monster drops whatever it was carrying
// ============================================================

static void drop_item_setup(void) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);
    protectEquipmentFromAcid();

    TEST_ASSERT(findFloorItem(FOOD, RATION) == NULL, "Floor must be clear of items before drop test");

    enum monsterBehaviorFlags excludeBehaviors = MONST_INVULNERABLE | MONST_IMMUNE_TO_WEAPONS;
    enum monsterAbilityFlags  excludeAbilities = MA_KAMIKAZE; // Pit bloats fail this test   
    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, excludeBehaviors, 0, excludeAbilities);
    monst = placeHostileMonster(monstType, targetPos);
    giveMonsterItem(monst, FOOD, RATION);
    setMonsterHealth(monst, 1);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean item_was_dropped(void) {
    TEST_ASSERT(findFloorItem(FOOD, RATION) != NULL,
                "the slain monster should have dropped its item");
    return !testFailed;
}

// ============================================================
// BR20491 — attacking a visible monster with a weapon forfeits the pure mage feat
// ============================================================

static void pure_mage_setup(void) {
    clearArena();
    theItem = givePlayerItem(WEAPON, SWORD, 0);
    equip(theItem);

    TEST_ASSERT(playerHasFeat(FEAT_PURE_MAGE), "player must start test with feat");
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monst = placeHostileMonster(MK_RAT, targetPos);
}

static boolean pure_mage_forfeited(void) {
    TEST_ASSERT(!playerHasFeat(FEAT_PURE_MAGE),
                "attacking with a weapon should forfeit the pure mage feat");
    return !testFailed;
}

// ============================================================
// BR89326 — attacking an unaware creature forfeits the paladin feat
// ============================================================

static void paladin_setup(void) {
    clearArena();
    TEST_ASSERT(playerHasFeat(FEAT_PALADIN), "player must start test with feat");
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monst = placeVisibleMonster(MK_TROLL, targetPos);
    makeMonsterUnaware(monst);
}

static boolean paladin_forfeited(void) {
    TEST_ASSERT(!playerHasFeat(FEAT_PALADIN),
                "attacking an unaware creature should forfeit the paladin feat");
    return !testFailed;
}

// ============================================================
// BR86360 — a staggering weapon knocks its target back one square
// ============================================================

static void stagger_setup(void) {
    clearArena();

    givePlayerRunicArmor(CHAIN_MAIL, A_RESPIRATION, 1, true); // Protection from paralysis gas

    theItem = givePlayerItem(WEAPON, MACE, 0);
    equip(theItem);
    protectEquipmentFromAcid();

    // Need a creature that won't die instantly or split
    enum monsterBehaviorFlags excludeBehaviors = MONST_INVULNERABLE | MONST_IMMOBILE | MONST_INANIMATE | MONST_GETS_TURN_ON_ACTIVATION;
    enum monsterAbilityFlags  excludeAbilities = MA_KAMIKAZE | MA_CLONE_SELF_ON_DEFEND | MA_HIT_STEAL_FLEE;
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, excludeBehaviors, 0, excludeAbilities);
    monst = placeHostileMonster(monstType, targetPos);
    setMonsterHealth(monst, 500);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
    spawnFeature(getPlayerPos(), DF_PARALYSIS_GAS_CLOUD_POTION);
}

static boolean stagger_step(void) {
    moveRight();
    if (monsterIsAlive(monst) && posEq(targetPos, monsterPosition(monst)) && turnNumber() < 1000) {
        setMonsterHealth(monst, 500);
        applyStatusToMonster(monst, STATUS_PARALYZED, 20);
        spawnFeature(getPlayerPos(), DF_PARALYSIS_GAS_CLOUD_POTION);
        return true;
    } 
    return false;
}

static boolean target_was_knocked_back(void) {
    TEST_ASSERT(monsterIsAlive(monst) && !posEq(targetPos, monsterPosition(monst)),
                       "a staggering weapon should push the target back one square");
    return !testFailed;
}

// ============================================================
// BR95398 — a monster that flees does so at a quarter health
// ============================================================

static void flee_setup(void) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);

    monstType = chooseMonsterTypeByProperty(BOLT_NONE, MONST_FLEES_NEAR_DEATH, 0, 0, 0);
    monst = placeHostileMonster(monstType, targetPos);
    // Drop it to exactly a quarter of its maximum health.
    damageMonster(monst, monst->info.maxHP - monst->info.maxHP / 4);
}

static boolean monster_fled(void) {
    TEST_ASSERT(monsterIsFleeing(monst),
                "a monster wounded to a quarter health should flee");
    return !testFailed;
}

// ============================================================
// BR74866 — a monster that divides splits in two when struck
// ============================================================

static void splitting_setup(void) {
    clearArena();
    protectEquipmentFromAcid();

    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, 0, MA_CLONE_SELF_ON_DEFEND, 0);
    monst = placeHostileMonster(monstType, posOffset(getPlayerPos(), 1, 0));
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
    monsterCountBefore = monsterCount();
}

static boolean monster_divided(void) {
    TEST_ASSERT(monsterCount() > monsterCountBefore,
                "striking a dividing monster should create a clone");
    return !testFailed;
}

// ============================================================
// BR82949 — a weakening attack weakens its victim
// ============================================================

static void weakening_setup(void) {
    clearArena();
    setPlayerHealth(500); // Don't let the player die
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, 0, MA_CAUSES_WEAKNESS, 0);
    monst = placeHostileMonster(monstType, targetPos);
    applyStatusToPlayer(STATUS_PARALYZED, 30);
}

static boolean player_was_weakened(void) {
    TEST_ASSERT(!isPlayerDead() && isPlayerWeakened(), "a weakening attack should weaken the player");
    return !testFailed;
}

// ============================================================
// BR61676 — an exploding attacker dies in the act of attacking
// ============================================================

static void kamikaze_setup(void) {
    clearArena();
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, 0, MA_KAMIKAZE, 0);
    monst = spawnMonster(monstType, targetPos);
    makeMonsterHostile(monst);
    revealCell(targetPos);
}

static boolean kamikaze_step(void) {
    restOneTurn();
    if (monsterIsAlive(monst)) return true;
    return false;
}

static boolean bloat_was_destroyed(void) {
    TEST_ASSERT(monsterIsDead(monst), "an exploding attacker should die when it attacks");
    return !testFailed;
}

// ============================================================
// BR57747 — killing a dragon earns the dragonslayer feat
// ============================================================

static void dragonslayer_setup(void) {
    clearArena();
    TEST_ASSERT(!playerHasFeat(FEAT_DRAGONSLAYER),"must begin test without the feat");
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monst = placeHostileMonster(MK_DRAGON, targetPos);
    setMonsterHealth(monst, 1);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean dragonslayer_earned(void) {
    TEST_ASSERT(playerHasFeat(FEAT_DRAGONSLAYER),
                "killing a dragon should earn the dragonslayer feat");
    return !testFailed;
}

// ============================================================
// BR86062 — a blow that kills the player ends the game
// ============================================================

static void player_death_setup(void) {
    clearArena();
    setPlayerHealth(1);
    applyStatusToPlayer(STATUS_PARALYZED, 30);
    targetPos = posOffset(getPlayerPos(), 1, 0);
    spawnMonster(MK_OGRE, targetPos);   // TODO: Random monsters
    revealCell(targetPos);
}

static boolean rest_once_step(void) {
    restOneTurn();
    return false;
}

static boolean game_ended(void) {
    TEST_ASSERT(rogue.gameHasEnded, "the player's death should end the game");
    return !testFailed;
}

// ============================================================
// BR11906 — an attacker with transference heals itself for a share of the damage
// ============================================================

// Positive test: a monster with MA_TRANSFERENCE heals itself when it attacks.
static void transference_heals_setup(void) {
    clearArena();
    applyStatusToPlayer(STATUS_PARALYZED, 30);
    monst = placeHostileMonster(MK_VAMPIRE_BAT, posOffset(getPlayerPos(), 1, 0));
    setMonsterHealth(monst, 1);
    healthBefore = monsterHealth(monst);
}

static boolean monster_was_healed(void) {
    TEST_ASSERT(isPlayerHurt() && monsterHealth(monst) > healthBefore,
                "transference attacker should heal itself when striking");
    return !testFailed;
}

// ============================================================
// BR94361 — immolation armor has a 10% chance when the player is struck to
// erupt in a burst of flame
// ============================================================

static void immolation_armor_setup(void) {
    clearArena();
    theItem = givePlayerRunicArmor(LEATHER_ARMOR, A_IMMOLATION, -3, true);
    protectEquipmentFromAcid();

    // Need a monster that attacks player repeatedly (melee) without fleeing or dying
    enum monsterBehaviorFlags excludeBehaviors = MONST_INANIMATE | MONST_IMMOBILE | MONST_GETS_TURN_ON_ACTIVATION | MONST_MAINTAINS_DISTANCE | MONST_RESTRICTED_TO_LIQUID;
    monstType = chooseMonsterTypeByProperty(BOLT_NONE, 0, excludeBehaviors, 0, MA_KAMIKAZE | MA_HIT_STEAL_FLEE);
    monst = placeHostileMonster(monstType, posOffset(getPlayerPos(), 1, 0));
    setPlayerHealth(100);
}

static boolean immolation_strikes_step(void) {
    setPlayerHealth(100);
    restOneTurn();
    if (messageWasShown("flames suddenly explode") || turnNumber() > 1000 || isPlayerDead()) return false;
    return true;
}

static boolean armor_erupts_in_flame(void) {
    TEST_ASSERT(!isPlayerDead() && messageWasShown("flames suddenly explode"),
                "immolation armor should erupt in flame when the wearer is struck");
    return !testFailed;
}

// ============================================================
// BR57646 — weapon runes other than slaying never trigger
//            against inanimate or invulnerable creatures
// ============================================================

static void br57646_prepare(enum monsterTypes victimType) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_ABSORPTION, 50, true); // Creatures can't hurt us

    // TODO: random rune selection (not slaying)
    giveAndEquipRunicWeapon(DAGGER, W_CONFUSION, 20, true);
    monst = placeHostileMonster(victimType, posOffset(getPlayerPos(), 1, 0));
    applyStatusToMonster(monst, STATUS_STUCK, 20);
}

static boolean br57646_step(void) {
    setMonsterHealth(monst, monsterMaxHealth(monst));
    applyStatusToMonster(monst, STATUS_STUCK, 20);

    moveRight();
    if (monsterHasStatus(monst, STATUS_CONFUSED) || turnNumber() > 100) return false;
    return true;
}

// Rune should fire against normal creatures
static void br57646_ordinary_setup(void) {
    // TODO: randomize monster
    br57646_prepare(MK_TROLL);
}

static boolean ordinary_rune_triggered(void) {
    TEST_ASSERT(monsterHasStatus(monst, STATUS_CONFUSED), "a confusion rune should trigger against an ordinary creature");
    return !testFailed;
}

// Negative test: a goblin totem is inanimate, so its rune never fires.
static void br57646_inanimate_setup(void) {
    br57646_prepare(MK_GOBLIN_TOTEM);
}

static boolean inanimate_rune_never_triggered(void) {
    // Never triggered, but monster is hurt (has been struck)
    TEST_ASSERT(!monsterHasStatus(monst, STATUS_CONFUSED) && monsterIsHurt(monst),
                "a confusion rune should never trigger against an inanimate creature");
    return !testFailed;
}

// Negative test: the Warden of Yendor is invulnerable, so its rune never fires.
static void br57646_invulnerable_setup(void) {
    br57646_prepare(MK_WARDEN_OF_YENDOR);
}

static boolean invulnerable_rune_never_triggered(void) {
    // Not triggered, and not hurt
    TEST_ASSERT(!monsterHasStatus(monst, STATUS_CONFUSED) && monsterIsHealthy(monst),
                "a confusion rune should never trigger against an invulnerable creature");
    return !testFailed;
}

const testCase tests_combat[] = {
    { "BR94361_immolation_armor_erupts_in_flame", immolation_armor_setup, immolation_strikes_step, armor_erupts_in_flame },
    { "BR65933_paralyzed_defender_always_hit",  paralyzed_target_setup,  attack_once_step,        target_was_wounded },
    { "BR66846_sleeping_defender_always_hit",   sleeping_target_setup,   attack_once_step,        sleeping_target_was_wounded },
    { "BR89965_struck_monster_wakes",           sleeping_target_setup,   attack_once_step,        struck_monster_woke },
    { "BR62243_attacking_alerts_monster",       unaware_target_setup,    attack_once_step,     monster_was_alerted },
    { "BR33297_sweeping_attack_hits_adjacent",  sweep_setup,             attack_once_step,              all_adjacent_were_hit },
    { "BR77279_weapon_immune_takes_no_damage",  weapon_immune_setup,     attack_once_step,        immune_target_unharmed },
    { "BR81065_invulnerable_takes_no_damage",   invulnerable_setup,      attack_once_step,        invulnerable_target_unharmed },
    { "BR76528_lethal_blow_kills_monster",      lethal_blow_setup,       attack_once_step,        monster_was_killed },
    { "BR31952_slain_monster_drops_item",       drop_item_setup,         attack_once_step,        item_was_dropped },
    { "BR20491_attack_forfeits_pure_mage",      pure_mage_setup,         attack_once_step,        pure_mage_forfeited },
    { "BR89326_attack_unaware_forfeits_paladin",paladin_setup,           attack_once_step,        paladin_forfeited },
    { "BR86360_staggering_weapon_knocks_back",  stagger_setup,           stagger_step,            target_was_knocked_back },
    { "BR95398_monster_flees_at_quarter_health",flee_setup,              NULL,                    monster_fled },
    { "BR74866_dividing_monster_splits",        splitting_setup,         attack_once_step,        monster_divided },
    { "BR82949_weakening_attack_weakens_player",weakening_setup,         wait_until_hurt,         player_was_weakened },
    { "BR61676_exploding_attacker_dies",        kamikaze_setup,          kamikaze_step,           bloat_was_destroyed },
    { "BR57747_killing_dragon_earns_feat",      dragonslayer_setup,      attack_once_step,        dragonslayer_earned },
    { "BR86062_player_death_ends_game",         player_death_setup,      rest_once_step,          game_ended },
    { "BR11906_transference_heals_on_attack",   transference_heals_setup, monster_attacks_step,   monster_was_healed },
    { "BR57646_rune_triggers_against_ordinary_creature", br57646_ordinary_setup, br57646_step, ordinary_rune_triggered },
    { "BR57646_rune_never_triggers_against_inanimate_creature", br57646_inanimate_setup, br57646_step, inanimate_rune_never_triggered },
    { "BR57646_rune_never_triggers_against_invulnerable_creature", br57646_invulnerable_setup, br57646_step, invulnerable_rune_never_triggered },
    { NULL, NULL, NULL, NULL }
};
