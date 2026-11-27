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

static void seized_target_setup(void) {
    clearArena();
    setTerrainAroundLoc(getPlayerPos(), MUD);
    setPlayerPos(getPlayerPos());
    targetPos = posOffset(getPlayerPos(), 1, 0);
    monst = placeHostileMonster(MK_BOG_MONSTER, targetPos);
    // A zero-accuracy attacker never lands a normal blow, so any hit proves
    // the seize auto-hit.
    monst->info.accuracy = 0;
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

// ============================================================
// BR65933 — an attack always hits a defender the attacker has seized
// ============================================================

static boolean br65933_hit;

static boolean seized_defender_step(void) {
    monsterAttacksPlayer(monst);               // the grab seizes the player
    br65933_hit = monsterAttacksPlayer(monst); // the next blow must hit
    return false;
}

static boolean seized_defender_was_hit(void) {
    TEST_ASSERT(playerIsSeized(),
                "the bog monster should have seized the player");
    TEST_ASSERT(br65933_hit,
                "an attack against a seized defender should always hit");
    return !testFailed;
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

    theItem = givePlayerPlainWeapon(MACE, 0);
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

// ============================================================
// BR78777 — a confusion weapon confuses the creature it strikes
// for a duration that grows with enchantment, never less than
// three turns.  A beached kraken cannot act, and its health is
// restored before every blow, so the only thing that varies is
// whether the rune fires.
// ============================================================

static int br78777_duration;
static int br78777_expected;
static int br78777_attempts;

static void br78777_prepare(int enchant) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_ABSORPTION, 50, true); // keep the player safe
    giveAndEquipRunicWeapon(DAGGER, W_CONFUSION, enchant, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(getPlayerPos(), 1, 0));
    br78777_duration = 0;
    br78777_attempts = 0;
}

static boolean br78777_step(void) {
    setMonsterHealth(monst, monsterMaxHealth(monst)); // keep the target alive to be struck again
    playerAttacksMonster(monst);
    if (monsterHasStatus(monst, STATUS_CONFUSED)) {
        br78777_duration = monsterStatusTurns(monst, STATUS_CONFUSED);
        return false; // runic fired, done
    }
    return ++br78777_attempts < 2000;
}

static boolean br78777_confusion_duration_is(void) {
    TEST_ASSERT(br78777_duration > 0, "the confusion rune should have triggered");
    TEST_ASSERT_EQ(br78777_duration, br78777_expected,
                   "a confusion weapon should confuse its target for three turns at low enchantment and longer as enchantment grows");
    return !testFailed;
}

static void br78777_enchant1_setup(void) {
    br78777_prepare(1);
    br78777_expected = 3; // 1 * 3 / 2 is below the three-turn minimum
}

static void br78777_enchant4_setup(void) {
    br78777_prepare(4);
    br78777_expected = 6; // 4 * 3 / 2
}

// ============================================================
// BR29616 — a creature able to steal and flee takes a random unequipped
// item from the player's pack when its blow lands (never while it is
// confused) and then flees; a weapon stack of more than three loses about
// half, while any other stack loses one item.
// ============================================================

// The pack holds a 15-dart weapon stack, a wielded dagger and worn armor.
// Stripping the food leaves the darts as the only steal candidate, and a
// stuck player is always hit, so both hit rolls the steal depends on land.
static void steal_weapon_stack_setup(void) {
    clearArena();
    removePackFood();
    applyStatusToPlayer(STATUS_STUCK, 100);
    monst = placeHostileMonster(MK_MONKEY, posOffset(getPlayerPos(), 1, 0));
    monsterAttacksPlayer(monst);
}

static boolean weapon_stack_loses_half_and_monkey_flees(void) {
    TEST_ASSERT_EQ(packUnitCount(WEAPON, DART), 7,
                   "a stolen weapon stack of more than three should lose about half (15 darts -> 7)");
    TEST_ASSERT(monsterIsFleeing(monst),
                "the creature should flee once it has stolen an item");
    TEST_ASSERT(messageWasShown("stole"),
                "the theft should be announced to the player");
    return !testFailed;
}

// Negative test: a confused creature never steals, even when its blow lands.
static void confused_monkey_steals_setup(void) {
    clearArena();
    removePackFood();
    applyStatusToPlayer(STATUS_STUCK, 100);
    monst = placeHostileMonster(MK_MONKEY, posOffset(getPlayerPos(), 1, 0));
    applyStatusToMonster(monst, STATUS_CONFUSED, 20);
    monsterAttacksPlayer(monst);
}

static boolean confused_monkey_steals_nothing(void) {
    TEST_ASSERT_EQ(packUnitCount(WEAPON, DART), 15,
                   "a confused creature should leave the pack untouched");
    TEST_ASSERT(!monsterIsFleeing(monst),
                "a confused creature should not flee after its blow lands");
    return !testFailed;
}

// Strip the darts too, so a stack of four potions is the only steal
// candidate: a non-weapon stack must lose exactly one item, not half.
static void steal_consumable_stack_setup(void) {
    clearArena();
    removePackFood();
    removePackItems(WEAPON, DART);
    givePlayerItem(POTION, POTION_LIFE, 0);
    givePlayerItem(POTION, POTION_LIFE, 0);
    givePlayerItem(POTION, POTION_LIFE, 0);
    givePlayerItem(POTION, POTION_LIFE, 0);
    applyStatusToPlayer(STATUS_STUCK, 100);
    monst = placeHostileMonster(MK_MONKEY, posOffset(getPlayerPos(), 1, 0));
    monsterAttacksPlayer(monst);
}

static boolean consumable_stack_loses_one(void) {
    TEST_ASSERT_EQ(packUnitCount(POTION, POTION_LIFE), 3,
                   "a stolen non-weapon stack should lose exactly one item (4 potions -> 3)");
    TEST_ASSERT(monsterIsFleeing(monst),
                "the creature should flee once it has stolen an item");
    return !testFailed;
}

// ============================================================
// BR89754 — attacking an acid-type creature corrodes the player's
// weapon, lowering its enchantment by one and announcing that the
// weapon weakens; a slaying weapon attacking its designated kind
// is exempt from this
// ============================================================

enum br89754_phase {
    BR89754_VS_MOUND,     // plain weapon against an acid mound
    BR89754_VS_JELLY,     // plain weapon against an acidic jelly
    BR89754_SLAYING_JELLY, // slaying weapon against the same jelly
    BR89754_DONE
};

static item *br89754_plainWeapon;
static item *br89754_slayWeapon;
static creature *br89754_acidMound;
static creature *br89754_jelly;
static enum br89754_phase br89754_phase;
static int br89754_plainBefore;
static int br89754_plainExpected;
static int br89754_afterMound;
static int br89754_attacks;
static boolean br89754_moundCorroded;
static boolean br89754_moundAnnounced;
static boolean br89754_jellyCorroded;
static boolean br89754_jellyAnnounced;
static boolean br89754_slayHit;

static void br89754_setup(void) {
    clearArena();
    br89754_plainBefore = 3;
    // The player starts with a plain dagger; drop it so that the pack holds
    // only the dagger this test hands out.
    removePackItems(WEAPON, DAGGER);
    // A plain dagger: a randomly rolled slaying runic aimed at the jelly
    // class would be exempt from the corrosion this test asserts on.
    br89754_plainWeapon = givePlayerPlainWeapon(DAGGER, br89754_plainBefore);
    equipItemFromPack(WEAPON, DAGGER);
    // Built up front, but only equipped once the plain dagger has corroded
    // against both acid creatures.  Acid mounds belong to no slaying class,
    // so the exemption is exercised on the acidic jelly (jelly class), which
    // corrodes weapons just as an acid mound does.
    br89754_slayWeapon = givePlayerRunicWeapon(SWORD, W_SLAYING, 5, true);
    setItemTargetClass(br89754_slayWeapon, vorpalEnemyClassForMonster(MK_ACID_JELLY));
    br89754_acidMound = placeHostileMonster(MK_ACID_MOUND, posOffset(getPlayerPos(), 1, 0));
    TEST_ASSERT(br89754_acidMound != NULL, "setup: the acid mound should spawn");
    TEST_ASSERT(equippedWeapon() == br89754_plainWeapon,
                "setup: the plain dagger should be the equipped weapon");
    putMonsterToSleep(br89754_acidMound); // a sleeping defender is always hit
    br89754_phase = BR89754_VS_MOUND;
    br89754_plainExpected = br89754_plainBefore;
    br89754_afterMound = br89754_plainBefore;
    br89754_attacks = 0;
    br89754_moundCorroded = false;
    br89754_moundAnnounced = false;
    br89754_jellyCorroded = false;
    br89754_jellyAnnounced = false;
    br89754_slayHit = false;
}

static boolean br89754_step(void) {
    switch (br89754_phase) {
        case BR89754_VS_MOUND:
            if (itemEnchant(br89754_plainWeapon) < br89754_plainExpected) {
                br89754_moundCorroded = true;
                br89754_moundAnnounced = messageWasShown("weakens");
                br89754_afterMound = itemEnchant(br89754_plainWeapon);
                clearMessages(); // each phase is judged on its own messages
                // Same corrosion test again, but against the jelly that the
                // slaying exemption below will be tried on.
                br89754_jelly = placeHostileMonster(MK_ACID_JELLY,
                                                     posOffset(getPlayerPos(), -1, 0));
                if (br89754_jelly == NULL) {
                    return false;
                }
                putMonsterToSleep(br89754_jelly);
                br89754_plainExpected = itemEnchant(br89754_plainWeapon);
                br89754_phase = BR89754_VS_JELLY;
                br89754_attacks = 0;
                return true;
            }
            if (br89754_attacks >= 8) {
                return false; // let verify report the missing corrosion
            }
            playerAttacksMonster(br89754_acidMound);
            br89754_attacks++;
            return true;

        case BR89754_VS_JELLY:
            if (itemEnchant(br89754_plainWeapon) < br89754_plainExpected) {
                br89754_jellyCorroded = true;
                br89754_jellyAnnounced = messageWasShown("weakens");
                clearMessages(); // judge the slaying weapon on its own messages
                equipItemFromPack(WEAPON, SWORD);
                putMonsterToSleep(br89754_jelly);
                br89754_phase = BR89754_SLAYING_JELLY;
                br89754_attacks = 0;
                return true;
            }
            if (br89754_attacks >= 8) {
                return false;
            }
            playerAttacksMonster(br89754_jelly);
            br89754_attacks++;
            return true;

        case BR89754_SLAYING_JELLY:
            if (!br89754_slayHit && br89754_attacks < 8) {
                br89754_slayHit = playerAttacksMonster(br89754_jelly);
                br89754_attacks++;
                return br89754_slayHit ? false : true;
            }
            return false;

        default:
            return false;
    }
}

static boolean br89754_corrodes_weapon_unless_slaying(void) {
    TEST_ASSERT(br89754_moundCorroded,
                "attacking an acid mound should corrode the plain weapon");
    TEST_ASSERT_EQ(br89754_afterMound, br89754_plainBefore - 1,
                   "the plain weapon's enchantment should drop by one against the acid mound");
    TEST_ASSERT(br89754_moundAnnounced,
                "the corrosion should announce that the weapon weakens");
    TEST_ASSERT(br89754_jellyCorroded,
                "the plain weapon should also corrode against the acidic jelly");
    TEST_ASSERT_EQ(itemEnchant(br89754_plainWeapon), br89754_plainBefore - 2,
                   "the plain weapon's enchantment should drop again against the acidic jelly");
    TEST_ASSERT(br89754_jellyAnnounced,
                "the second corrosion should announce that the weapon weakens too");
    TEST_ASSERT(br89754_slayHit,
                "the slaying weapon should land a blow on the sleeping acidic jelly");
    TEST_ASSERT_EQ(itemEnchant(br89754_slayWeapon), 5,
                   "a slaying weapon attacking its designated kind should not corrode");
    TEST_ASSERT(!messageWasShown("weakens"),
                "the exempt slaying weapon should not report weakening");
    return !testFailed;
}

// ============================================================
// BR41331 — reaping recharges the player's staffs, wands, and charms by a
// random amount up to the damage dealt times the reaping bonus, never more
// than the victim's remaining health
// ============================================================

// The charm's countdown is reduced by exactly the amount reaping rolls, so
// the change in its charge count is a direct reading of the recharge.
#define BR41331_CAP_TRIALS 30
#define BR41331_DAMAGE_TRIALS 16

static item *br41331_charm;
static int br41331_reaping;
static int br41331_chargesAtStart;
static int br41331_targetHealth;
static int br41331_trialLimit;
static int br41331_trialsDone;
static int br41331_hits;
static int br41331_positiveRecharges;
static int br41331_overCap;
static int br41331_rechargeExceededDamage;
static int br41331_maxRecharge;

static void br41331_prepare(int targetHealth, int trialLimit) {
    clearArena();
    setPlayerNutrition(2000);

    givePlayerItem(RING, RING_REAPING, 4);
    equipItemFromPack(RING, RING_REAPING);
    br41331_reaping = playerReapingBonus();

    br41331_charm = givePlayerItem(CHARM, CHARM_HEALTH, 1);
    applyPackItem(CHARM, CHARM_HEALTH);
    br41331_chargesAtStart = itemCharges(br41331_charm);

    br41331_targetHealth = targetHealth;
    br41331_trialLimit = trialLimit;
    br41331_trialsDone = 0;
    br41331_hits = 0;
    br41331_positiveRecharges = 0;
    br41331_overCap = 0;
    br41331_rechargeExceededDamage = 0;
    br41331_maxRecharge = 0;
}

static void br41331_cap_setup(void) {
    br41331_prepare(1, BR41331_CAP_TRIALS);
}

static void br41331_damage_setup(void) {
    br41331_prepare(1000, BR41331_DAMAGE_TRIALS);
}

static boolean br41331_attack_step(void) {
    if (br41331_trialsDone >= br41331_trialLimit) {
        return false;
    }

    clearMonsters();
    creature *victim = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    setMonsterHealth(victim, br41331_targetHealth);
    // A sleeping victim is always hit, and the starting dagger's sneak bonus
    // makes every blow far larger than a single hit point.
    putMonsterToSleep(victim);

    int healthBefore = monsterHealth(victim);
    int chargesBefore = itemCharges(br41331_charm);
    boolean hit = playerAttacksMonster(victim);
    int healthAfter = monsterHealth(victim);
    int chargesAfter = itemCharges(br41331_charm);

    if (hit) {
        int damage = healthBefore - healthAfter;
        int recharged = chargesBefore - chargesAfter;
        int damageCap = (damage < br41331_targetHealth ? damage : br41331_targetHealth)
                        * br41331_reaping;

        br41331_hits++;
        if (recharged > 0) {
            br41331_positiveRecharges++;
        }
        if (recharged > damageCap) {
            br41331_overCap++;
        }
        if (recharged > damage) {
            br41331_rechargeExceededDamage++;
        }
        if (recharged > br41331_maxRecharge) {
            br41331_maxRecharge = recharged;
        }
    }

    br41331_trialsDone++;
    return br41331_trialsDone < br41331_trialLimit;
}

static boolean br41331_reaping_is_capped_by_victim_health(void) {
    TEST_ASSERT_EQ(br41331_reaping, 4,
                   "a +4 reaping ring should give a reaping bonus of 4");
    TEST_ASSERT(br41331_chargesAtStart > 0,
                "setup: using the charm should start its recharge countdown");
    TEST_ASSERT(br41331_hits > 0,
                "setup: the sleeping victims should be hit");
    TEST_ASSERT_EQ(br41331_overCap, 0,
                   "reaping must never recharge more than the victim's remaining health times the bonus");
    TEST_ASSERT(br41331_positiveRecharges > 0,
                "reaping should recharge the charm at least once");
    TEST_ASSERT(br41331_maxRecharge <= br41331_reaping,
                "a victim left with one hit point cannot fund more recharge than the reaping bonus");
    return !testFailed;
}

static boolean br41331_reaping_is_bounded_by_damage(void) {
    TEST_ASSERT_EQ(br41331_reaping, 4,
                   "a +4 reaping ring should give a reaping bonus of 4");
    TEST_ASSERT(br41331_chargesAtStart > 0,
                "setup: using the charm should start its recharge countdown");
    TEST_ASSERT(br41331_hits > 0,
                "setup: the sleeping victims should be hit");
    TEST_ASSERT_EQ(br41331_overCap, 0,
                   "reaping must never recharge more than damage dealt times the reaping bonus");
    TEST_ASSERT(br41331_positiveRecharges > 0,
                "reaping should recharge the charm at least once");
    TEST_ASSERT(br41331_rechargeExceededDamage > 0,
                "with a x4 reaping bonus, some recharge should exceed the blow's base damage");
    return !testFailed;
}

// ============================================================
// BR25647 — attacking one of your own allies while you are not
// confused makes that monster stop being your ally.
//
// The ally has far more health than one blow can remove, so the
// only thing the hit can change is the alliance.  Attacks are
// repeated until one lands, making the test independent of the
// accuracy roll; a missed blow must not break the alliance
// either.  The confused control performs the same attack but
// expects the alliance to survive.
// ============================================================

static creature *br25647_ally;
static boolean br25647_landed;

static void br25647_unconfused_setup(void) {
    clearArena();
    br25647_ally = spawnMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    TEST_ASSERT(br25647_ally != NULL, "setup: the ally should spawn");
    if (br25647_ally != NULL) {
        makeMonsterAlly(br25647_ally);
        setMonsterHealth(br25647_ally, 1000);
    }
    TEST_ASSERT(br25647_ally != NULL && monsterIsAlly(br25647_ally),
                "precondition: the monster should be your ally");
    TEST_ASSERT(!isPlayerConfused(),
                "precondition: you should not be confused");
}

static void br25647_confused_setup(void) {
    br25647_unconfused_setup();
    applyStatusToPlayer(STATUS_CONFUSED, 100);
    TEST_ASSERT(isPlayerConfused(),
                "precondition: you should be confused");
}

static boolean br25647_attack_once_step(void) {
    br25647_landed = false;
    if (br25647_ally == NULL) {
        return false;
    }
    for (int attempt = 0; attempt < 100
            && !br25647_landed && monsterIsAlive(br25647_ally); attempt++) {
        br25647_landed = playerAttacksMonster(br25647_ally);
    }
    return false;
}

static boolean br25647_unconfused_attack_ends_alliance(void) {
    if (br25647_ally == NULL) {
        return false;
    }
    TEST_ASSERT(br25647_landed,
                "a melee attack against the adjacent ally should land");
    TEST_ASSERT(monsterIsAlive(br25647_ally),
                "the ally should survive the blow under test");
    TEST_ASSERT(!monsterIsAlly(br25647_ally),
                "an ally you attack while not confused should stop being your ally");
    return !testFailed;
}

static boolean br25647_confused_attack_keeps_alliance(void) {
    if (br25647_ally == NULL) {
        return false;
    }
    TEST_ASSERT(isPlayerConfused(),
                "precondition: you should still be confused");
    TEST_ASSERT(br25647_landed,
                "a melee attack against the adjacent ally should land");
    TEST_ASSERT(monsterIsAlive(br25647_ally),
                "the ally should survive the blow under test");
    TEST_ASSERT(monsterIsAlly(br25647_ally),
                "an ally you attack while confused should remain your ally");
    return !testFailed;
}

// ============================================================
// BR34465 — a stuck defender is always hit, even though stuck does not
// qualify for triple damage (only surprise, sleep, and paralysis do).
//
// The attacker is a spectral sword, whose 1-1 damage makes the multiplier
// observable: a normal blow costs 1 health and a tripled blow costs 3.  Its
// accuracy is set to zero, so the unhindered control must miss while the
// stuck and paralyzed blows land through their auto-hit paths.
// ============================================================

static creature *br34465_attacker;

static void br34465_setup(void) {
    clearArena();
    clearMonsters();
    takeOffArmor();
    setPlayerHealth(100);
    br34465_attacker = placeHostileMonster(MK_SPECTRAL_IMAGE,
                                           posOffset(getPlayerPos(), 1, 0));
    setMonsterAccuracy(br34465_attacker, 0);
}

static boolean br34465_verify(void) {
    int healthBefore;

    TEST_ASSERT(br34465_attacker != NULL, "spectral sword spawned");
    TEST_ASSERT(!playerHasStatus(STATUS_STUCK) && !playerHasStatus(STATUS_PARALYZED),
                "control: the defender starts unhindered");

    // A zero-accuracy blow cannot land against an unhindered defender, so
    // any later hit on the stuck defender proves the auto-hit.
    healthBefore = playerHealth();
    TEST_ASSERT(!monsterAttacksPlayer(br34465_attacker),
                "control: a zero-accuracy attacker misses an unhindered defender");
    TEST_ASSERT_EQ(playerHealth(), healthBefore,
                   "control: a missed blow costs no health");

    applyStatusToPlayer(STATUS_STUCK, 100);
    healthBefore = playerHealth();
    TEST_ASSERT(monsterAttacksPlayer(br34465_attacker),
                "a stuck defender is always hit");
    TEST_ASSERT_EQ(healthBefore - playerHealth(), 1,
                   "a stuck defender takes normal damage, not triple damage");

    // Paralysis, unlike stuckness, earns the triple-damage multiplier.
    removeStatusFromPlayer(STATUS_STUCK);
    applyStatusToPlayer(STATUS_PARALYZED, 100);
    healthBefore = playerHealth();
    TEST_ASSERT(monsterAttacksPlayer(br34465_attacker),
                "control: a paralyzed defender is hit");
    TEST_ASSERT_EQ(healthBefore - playerHealth(), 3,
                   "control: a paralyzed defender takes triple damage");
    return !testFailed;
}

// ============================================================
// BR30875 — when the attacker is directly adjacent, the attacker's
// square joins the connected mass, so a splitting creature can leave
// its clone on the far side of the attacker.
//
// The arena is a one-square-wide corridor: floor behind the player, the
// player's square, and the jelly directly east.  Every other neighbour
// of the pair is granite, and the player's square blocks the jelly's
// own west neighbour.  Without the attacker's square in the mass there
// is no eligible square at all and the jelly could not split; with it,
// the only candidate is the corridor square behind the player.
// ============================================================

static pos br30875_playerPos;
static pos br30875_behindPos;
static pos br30875_jellyPos;
static creature *br30875_jelly;

static void br30875_setup(void) {
    clearMonsters();
    br30875_playerPos = getPlayerPos();
    br30875_behindPos = posOffset(br30875_playerPos, -1, 0);
    br30875_jellyPos = posOffset(br30875_playerPos, 1, 0);

    // Wall off the whole neighbourhood, then carve the corridor.
    setArenaRadius(2, GRANITE);
    setTerrain(br30875_behindPos, FLOOR);
    setTerrain(br30875_playerPos, FLOOR);
    setTerrain(br30875_jellyPos, FLOOR);
    setPlayerPos(br30875_playerPos);

    br30875_jelly = placeHostileMonster(MK_PINK_JELLY, br30875_jellyPos);
    // A paralyzed defender is hit automatically, so the blow that
    // triggers the split cannot miss.
    applyStatusToMonster(br30875_jelly, STATUS_PARALYZED, 100);
}

static boolean br30875_step(void) {
    TEST_ASSERT(br30875_jelly != NULL,
                "setup: the jelly should be standing in the corridor");
    TEST_ASSERT(!cellObstructsPassage(br30875_behindPos),
                "setup: the corridor should run one square behind the player");
    TEST_ASSERT(cellObstructsPassage(posOffset(br30875_playerPos, 0, 1))
                && cellObstructsPassage(posOffset(br30875_playerPos, 0, -1)),
                "setup: the corridor should be one square wide beside the player");
    TEST_ASSERT(cellObstructsPassage(posOffset(br30875_jellyPos, 1, 0))
                && cellObstructsPassage(posOffset(br30875_jellyPos, 0, 1))
                && cellObstructsPassage(posOffset(br30875_jellyPos, 0, -1)),
                "setup: the jelly should have no free neighbour of its own");
    TEST_ASSERT(playerAttacksMonster(br30875_jelly),
                "the blow against the paralyzed jelly should land");
    return false;
}

static boolean br30875_verify(void) {
    int clonesBehind = 0;

    TEST_ASSERT_EQ(countMonstersOfType(MK_PINK_JELLY), 2,
                   "the struck jelly should have split into two");
    TEST_ASSERT_POS_EQ(monsterPosition(br30875_jelly), br30875_jellyPos,
                       "the original jelly should stay on the square that was struck");
    for (int i = 0; i < monsterCount(); i++) {
        creature *m = monsterAtIndex(i);
        if (m == NULL) {
            continue;
        }
        pos p = monsterPosition(m);
        if (p.x == br30875_behindPos.x && p.y == br30875_behindPos.y) {
            clonesBehind++;
        }
    }
    TEST_ASSERT_EQ(clonesBehind, 1,
                   "the clone should appear behind the player, the only square the attacker's inclusion makes eligible");
    return !testFailed;
}

const testCase tests_combat[] = {
    { "BR94361_immolation_armor_erupts_in_flame", immolation_armor_setup, immolation_strikes_step, armor_erupts_in_flame },
    { "BR65933_seized_defender_always_hit",     seized_target_setup,     seized_defender_step,    seized_defender_was_hit },
    { "BR34465_stuck_defender_hit_without_triple_damage", br34465_setup, NULL, br34465_verify },
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
    { "BR30875_clone_appears_behind_adjacent_attacker", br30875_setup, br30875_step, br30875_verify },
    { "BR82949_weakening_attack_weakens_player",weakening_setup,         wait_until_hurt,         player_was_weakened },
    { "BR61676_exploding_attacker_dies",        kamikaze_setup,          kamikaze_step,           bloat_was_destroyed },
    { "BR57747_killing_dragon_earns_feat",      dragonslayer_setup,      attack_once_step,        dragonslayer_earned },
    { "BR86062_player_death_ends_game",         player_death_setup,          rest_once_step,          game_ended },
    { "BR29616_steal_takes_half_a_weapon_stack", steal_weapon_stack_setup,    NULL,                    weapon_stack_loses_half_and_monkey_flees },
    { "BR29616_confused_creature_steals_nothing", confused_monkey_steals_setup, NULL,                  confused_monkey_steals_nothing },
    { "BR29616_steal_takes_one_from_other_stack", steal_consumable_stack_setup, NULL,                   consumable_stack_loses_one },
    { "BR11906_transference_heals_on_attack",   transference_heals_setup, monster_attacks_step,   monster_was_healed },
    { "BR57646_rune_triggers_against_ordinary_creature", br57646_ordinary_setup, br57646_step, ordinary_rune_triggered },
    { "BR57646_rune_never_triggers_against_inanimate_creature", br57646_inanimate_setup, br57646_step, inanimate_rune_never_triggered },
    { "BR57646_rune_never_triggers_against_invulnerable_creature", br57646_invulnerable_setup, br57646_step, invulnerable_rune_never_triggered },
    { "BR78777_confusion_weapon_minimum_duration", br78777_enchant1_setup, br78777_step, br78777_confusion_duration_is },
    { "BR78777_confusion_weapon_duration_grows_with_enchant", br78777_enchant4_setup, br78777_step, br78777_confusion_duration_is },

    { "BR89754_acid_corrodes_weapon_slaying_exempt", br89754_setup, br89754_step, br89754_corrodes_weapon_unless_slaying },
    { "BR41331_reaping_capped_by_victim_health", br41331_cap_setup, br41331_attack_step, br41331_reaping_is_capped_by_victim_health },
    { "BR41331_reaping_bounded_by_damage", br41331_damage_setup, br41331_attack_step, br41331_reaping_is_bounded_by_damage },

    { "BR25647_unconfused_attack_ends_alliance", br25647_unconfused_setup, br25647_attack_once_step, br25647_unconfused_attack_ends_alliance },
    { "BR25647_confused_attack_keeps_alliance",  br25647_confused_setup,   br25647_attack_once_step, br25647_confused_attack_keeps_alliance },
    { NULL, NULL, NULL, NULL }
};
