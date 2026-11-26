// Tests derived from rules/04-monsters.md.
//
// These cover the deterministic, headless-observable monster rules: what hides
// a creature from an observer, who is willing to attack whom, allegiance
// (teammates and enemies), which monsters you know the location of, the
// terrain a monster refuses to enter, a beached liquid creature, a water-bound
// grappler staying submerged on the turn it grabs, whether a never-sleeping
// monster starts awake, which
// targets a monster is allowed to cast at, the order a monster tries its
// steps in when walking toward a destination, how a wandering monster
// travels between its checkpoints, how an ally idles when it is close
// to you and can see you, how an ally told not to follow you holds its
// post while still chasing enemies within its range, how a monster's
// timed conditions count down once per turn, how a permanently fleeing
// thief that loses its stolen item returns to normal behaviour, how an
// always-hunting monster that has given up on the scent trail paths toward
// you, the randomly chosen kind name a visible monster is displayed under
// while you hallucinate, and the allegiance of the members of a horde flagged
// as allied with you.
//
// Omitted are everything that depends on the particular layout of generated
// levels or on most horde generation, random chances (mutations, item drops,
// detection rolls, splitting, summoning counts), spell/bolt selection,
// messages and death text, and any rule that needs a screen or is otherwise
// not reproducible headlessly.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// ---------------------------------------------
// Shared helpers
// ---------------------------------------------

// Put a monster of the given kind on a chosen patch of terrain at a spot
// relative to the player.
static creature *monsterOn(enum monsterTypes type, int dx, int dy, short terrain) {
    pos p = posOffset(getPlayerPos(), dx, dy);
    setTerrain(p, terrain);
    return spawnMonster(type, p);
}

// Re-lay the player's own tile so its terrain is known.  setTerrain clears the
// whole cell, including the HAS_PLAYER flag, so the player is replaced on it.
static void standPlayerOn(short terrain) {
    pos p = getPlayerPos();
    setTerrain(p, terrain);
    setPlayerPos(p);
}

static creature *subject;       // the monster a rule is about
static creature *otherMonster;  // a second monster, when a rule needs one
static creature *leaderMonster;
static pos markedPos;
static int markedHealth;

// ---------------------------------------------
// BR92702 — a creature is hidden if dormant, invisible or submerged
// ---------------------------------------------

static void dormant_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    makeMonsterDormant(subject);
}

static boolean dormant_monster_is_hidden(void) {
    TEST_ASSERT(monsterIsHiddenFrom(subject, &player),
                "a dormant monster should be hidden from you");
    return !testFailed;
}

static void invisible_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    applyStatusToMonster(subject, STATUS_INVISIBLE, 100);
}

static boolean invisible_monster_is_hidden(void) {
    TEST_ASSERT(monsterIsHiddenFrom(subject, &player),
                "an invisible monster should be hidden from you");
    return !testFailed;
}

static void submerged_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, DEEP_WATER);
    setMonsterSubmerged(subject, true);
}

static boolean submerged_monster_is_hidden(void) {
    TEST_ASSERT(monsterIsHiddenFrom(subject, &player),
                "a submerged monster should be hidden from a dry observer");
    return !testFailed;
}

// ---------------------------------------------
// BR43781 — allies and followers are never hidden from each other
// ---------------------------------------------

static void invisible_ally_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    makeMonsterAlly(subject);
    applyStatusToMonster(subject, STATUS_INVISIBLE, 100);
    setMonsterSubmerged(subject, true);
}

static boolean ally_is_not_hidden(void) {
    TEST_ASSERT(!monsterIsHiddenFrom(subject, &player),
                "an ally should never be hidden from you");
    return !testFailed;
}

static void invisible_follower_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    leaderMonster = monsterOn(MK_TROLL, 5, 0, FLOOR);
    subject = monsterOn(MK_RAT, 6, 0, FLOOR);
    makeMonsterFollow(subject, leaderMonster);
    applyStatusToMonster(subject, STATUS_INVISIBLE, 100);
}

static boolean follower_is_not_hidden(void) {
    TEST_ASSERT(!monsterIsHiddenFrom(subject, leaderMonster),
                "a follower should never be hidden from its leader");
    return !testFailed;
}

// ---------------------------------------------
// BR97021 — submersion only hides you from observers not in deep water
// ---------------------------------------------

static void submerged_with_dry_observer_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, DEEP_WATER);
    setMonsterSubmerged(subject, true);
    otherMonster = monsterOn(MK_RAT, 6, 0, FLOOR);
}

static boolean submerged_hidden_from_dry_observer(void) {
    TEST_ASSERT(monsterIsHiddenFrom(subject, otherMonster),
                "a submerged monster should be hidden from a dry observer");
    return !testFailed;
}

static void submerged_with_water_observer_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, DEEP_WATER);
    setMonsterSubmerged(subject, true);
    otherMonster = monsterOn(MK_RAT, 6, 0, DEEP_WATER);
}

static boolean submerged_seen_from_deep_water(void) {
    TEST_ASSERT(!monsterIsHiddenFrom(subject, otherMonster),
                "two creatures in deep water should see each other");
    return !testFailed;
}

static void submerged_with_levitating_observer_setup(void) {
    submerged_with_water_observer_setup();
    applyStatusToMonster(otherMonster, STATUS_LEVITATING, 100);
}

static boolean levitating_observer_cannot_see_submerged(void) {
    TEST_ASSERT(monsterIsHiddenFrom(subject, otherMonster),
                "levitation should not let an observer see under the surface");
    return !testFailed;
}

// ---------------------------------------------
// BR63373 — telepathy reveals every non-inanimate monster
// ---------------------------------------------

static void telepathy_living_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    applyStatusToPlayer(STATUS_TELEPATHIC, 100);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
}

static boolean telepathy_reveals_living_monster(void) {
    TEST_ASSERT(monsterIsRevealedToPlayer(subject),
                "telepathy should reveal a living monster");
    return !testFailed;
}

static void telepathy_inanimate_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    applyStatusToPlayer(STATUS_TELEPATHIC, 100);
    subject = monsterOn(MK_GOBLIN_TOTEM, 5, 0, FLOOR);
}

static boolean telepathy_skips_inanimate_monster(void) {
    TEST_ASSERT(!monsterIsRevealedToPlayer(subject),
                "telepathy should not reveal an inanimate monster");
    return !testFailed;
}

// ---------------------------------------------
// BR44440 — an entranced monster is always revealed
// ---------------------------------------------

static void entranced_revealed_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    applyStatusToMonster(subject, STATUS_ENTRANCED, 100);
}

static boolean entranced_monster_is_revealed(void) {
    TEST_ASSERT(monsterIsRevealedToPlayer(subject),
                "an entranced monster should always be revealed");
    return !testFailed;
}

// ---------------------------------------------
// BR43957 — you know a monster's location only when it is not hidden
// from you and either its square is visible or it has been revealed
// to you.  The sidebar lists exactly the monsters whose location you
// know, so it is the observable.
// ---------------------------------------------

static void known_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_OGRE, 1, 0, FLOOR);
    otherMonster = monsterOn(MK_KOBOLD, 0, 1, FLOOR);
    revealCell(monsterPosition(subject));
    revealCell(monsterPosition(otherMonster));
    applyStatusToMonster(otherMonster, STATUS_INVISIBLE, 100);
}

static boolean visible_monster_known_hidden_monster_unknown(void) {
    TEST_ASSERT(sidebarContains("Ogre"),
                "a monster on a visible square should be known and listed in the sidebar");
    TEST_ASSERT(!sidebarContains("Kobold"),
                "a hidden monster should be unknown even when its square is visible");
    return !testFailed;
}

static void distant_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_GOBLIN, 5, 0, FLOOR);
}

static boolean distant_monster_unknown(void) {
    TEST_ASSERT(!cellIsVisible(monsterPosition(subject)),
                "precondition: the goblin's square is not visible");
    TEST_ASSERT(!sidebarContains("Goblin"),
                "a monster whose square is neither visible nor revealed should be unknown");
    return !testFailed;
}

static void revealed_distant_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    applyStatusToPlayer(STATUS_TELEPATHIC, 100);
    subject = monsterOn(MK_GOBLIN, 5, 0, FLOOR);
}

static boolean revealed_distant_monster_known(void) {
    TEST_ASSERT(!cellIsVisible(monsterPosition(subject)),
                "precondition: revealing the monster does not make its square visible");
    TEST_ASSERT(sidebarContains("Goblin"),
                "a revealed monster should be known even though its square is not visible");
    return !testFailed;
}

static void hidden_revealed_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    applyStatusToPlayer(STATUS_TELEPATHIC, 100);
    subject = monsterOn(MK_KOBOLD, 1, 0, FLOOR);
    revealCell(monsterPosition(subject));
    applyStatusToMonster(subject, STATUS_INVISIBLE, 100);
}

static boolean hidden_monster_stays_unknown_when_revealed(void) {
    TEST_ASSERT(cellIsVisible(monsterPosition(subject)),
                "precondition: the kobold's square is visible");
    TEST_ASSERT(monsterIsRevealedToPlayer(subject),
                "precondition: telepathy has revealed the kobold");
    TEST_ASSERT(!sidebarContains("Kobold"),
                "a hidden monster should be unknown even when it has been revealed");
    return !testFailed;
}

// ---------------------------------------------
// BR78631 — an entranced monster attacks any non-ally, including you
// ---------------------------------------------

static void entranced_attacker_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    applyStatusToMonster(subject, STATUS_ENTRANCED, 100);
    otherMonster = monsterOn(MK_TROLL, 6, 0, FLOOR);
}

static boolean entranced_attacks_non_ally(void) {
    TEST_ASSERT(monsterCouldAttack(subject, otherMonster),
                "an entranced monster should attack a non-ally");
    return !testFailed;
}

static boolean entranced_attacks_player(void) {
    TEST_ASSERT(monsterCouldAttack(subject, &player),
                "an entranced monster should attack you");
    return !testFailed;
}

// ---------------------------------------------
// BR44999 — you do not attack an ally unless it is discordant
// ---------------------------------------------

static void ally_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    makeMonsterAlly(subject);
}

static boolean player_spares_ally(void) {
    TEST_ASSERT(!monsterCouldAttack(&player, subject),
                "you should not attack an ally");
    return !testFailed;
}

static void discordant_ally_setup(void) {
    ally_setup();
    applyStatusToMonster(subject, STATUS_DISCORDANT, 100);
}

static boolean player_attacks_discordant_ally(void) {
    TEST_ASSERT(monsterCouldAttack(&player, subject),
                "you may attack a discordant ally");
    return !testFailed;
}

// ---------------------------------------------
// BR45066 — allies do not attack an entranced creature
// ---------------------------------------------

static void ally_and_entranced_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    makeMonsterAlly(subject);
    otherMonster = monsterOn(MK_TROLL, 6, 0, FLOOR);
    applyStatusToMonster(otherMonster, STATUS_ENTRANCED, 100);
}

static boolean ally_spares_entranced_creature(void) {
    TEST_ASSERT(!monsterCouldAttack(subject, otherMonster),
                "an ally should not attack an entranced creature");
    return !testFailed;
}

// ---------------------------------------------
// BR45346 — nobody attacks a captive or a dying creature
// ---------------------------------------------

static void captive_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    otherMonster = monsterOn(MK_RAT, 6, 0, FLOOR);
    makeMonsterCaptive(otherMonster);
}

// BR45346's own setup: the captive pair, plus a creature that is already
// dying and a healthy control each attacker is willing to face, so the
// refusals in the verify step cannot pass vacuously.
static creature *allyControl;   // your ally — a plain monster will attack it
static creature *wildControl;   // a free creature — you will attack it
static creature *dyingMonster;

static void nobody_attacks_setup(void) {
    captive_setup();
    allyControl = monsterOn(MK_RAT, 7, 0, FLOOR);
    makeMonsterAlly(allyControl);
    wildControl = monsterOn(MK_RAT, 7, 1, FLOOR);
    dyingMonster = monsterOn(MK_RAT, 7, 2, FLOOR);
    makeMonsterDying(dyingMonster);
}

static boolean nobody_attacks_a_captive(void) {
    // Positive controls: both attackers are willing to attack a healthy
    // creature, so the refusals below come from the captive/dying state.
    TEST_ASSERT(monsterCouldAttack(subject, allyControl),
                "a monster should attack your ally when nothing stops it");
    TEST_ASSERT(monsterCouldAttack(&player, wildControl),
                "you should attack a free creature when nothing stops it");
    TEST_ASSERT(!monsterCouldAttack(subject, otherMonster),
                "a monster should not attack a captive");
    TEST_ASSERT(!monsterCouldAttack(&player, otherMonster),
                "you should not attack a captive");
    TEST_ASSERT(!monsterCouldAttack(subject, dyingMonster),
                "a monster should not attack a creature that is already dying");
    TEST_ASSERT(!monsterCouldAttack(&player, dyingMonster),
                "you should not attack a creature that is already dying");
    return !testFailed;
}

// ---------------------------------------------
// BR54066 — a discordant creature attacks anything, even its own kind
// ---------------------------------------------

static void discordant_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    applyStatusToMonster(subject, STATUS_DISCORDANT, 100);
    otherMonster = monsterOn(MK_RAT, 6, 0, FLOOR);
}

static boolean discordant_attacks_own_kind(void) {
    TEST_ASSERT(monsterCouldAttack(subject, otherMonster),
                "a discordant creature should attack its own kind");
    return !testFailed;
}

static boolean discordant_attacks_player(void) {
    TEST_ASSERT(monsterCouldAttack(subject, &player),
                "a discordant creature should attack you");
    return !testFailed;
}

// ---------------------------------------------
// BR45704 — captives are never enemies, discord makes enemies
// ---------------------------------------------

static boolean captive_is_never_enemy(void) {
    TEST_ASSERT(!monstersAreFoesOf(subject, otherMonster),
                "a captive should not be an enemy");
    TEST_ASSERT(!monstersAreFoesOf(otherMonster, subject),
                "a captive should not be an enemy");
    return !testFailed;
}

static void discordant_ally_pair_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    makeMonsterAlly(subject);
    applyStatusToMonster(subject, STATUS_DISCORDANT, 100);
    otherMonster = monsterOn(MK_RAT, 6, 0, FLOOR);
    makeMonsterAlly(otherMonster);
}

static boolean discord_makes_mutual_enemies(void) {
    TEST_ASSERT(monstersAreFoesOf(subject, otherMonster),
                "discord should make creatures enemies");
    return !testFailed;
}

static void opposite_sides_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR); // hostile
    otherMonster = monsterOn(MK_RAT, 6, 0, FLOOR);
    makeMonsterAlly(otherMonster);
}

static boolean opposite_sides_are_enemies(void) {
    TEST_ASSERT(monstersAreFoesOf(subject, &player),
                "a hostile monster should be your enemy");
    TEST_ASSERT(!monstersAreFoesOf(otherMonster, &player),
                "an ally should not be your enemy");
    return !testFailed;
}

// ---------------------------------------------
// BR45522 — followers of one leader, allies and you are teammates
// ---------------------------------------------

static boolean you_and_ally_are_teammates(void) {
    TEST_ASSERT(monstersAreAlliesOf(&player, subject),
                "you and an ally should be teammates");
    return !testFailed;
}

static void ally_pair_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    makeMonsterAlly(subject);
    otherMonster = monsterOn(MK_RAT, 6, 0, FLOOR);
    makeMonsterAlly(otherMonster);
}

static boolean allies_are_teammates(void) {
    TEST_ASSERT(monstersAreAlliesOf(subject, otherMonster),
                "allies should be teammates");
    return !testFailed;
}

static void followers_of_one_leader_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    leaderMonster = monsterOn(MK_TROLL, 5, 0, FLOOR);
    subject = monsterOn(MK_RAT, 6, 0, FLOOR);
    otherMonster = monsterOn(MK_RAT, 7, 0, FLOOR);
    makeMonsterFollow(subject, leaderMonster);
    makeMonsterFollow(otherMonster, leaderMonster);
}

static boolean followers_are_teammates(void) {
    TEST_ASSERT(monstersAreAlliesOf(subject, otherMonster),
                "followers of one leader should be teammates");
    return !testFailed;
}

// The negative side of the rule: creatures that share no leader and no
// allegiance are NOT teammates.  Two monsters that follow nobody and are
// nobody's ally form one pair; two followers led by different leaders form
// the other.  Either pair would read as teammates if the rule leaked — for
// example if any two followers counted together regardless of who leads them.
static creature *otherLeaderMonster;
static creature *neutralOne;
static creature *neutralTwo;

static void no_shared_leader_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    neutralOne = monsterOn(MK_RAT, 5, 0, FLOOR);
    neutralTwo = monsterOn(MK_RAT, 6, 0, FLOOR);
    leaderMonster = monsterOn(MK_TROLL, 7, 0, FLOOR);
    otherLeaderMonster = monsterOn(MK_OGRE, 8, 0, FLOOR);
    subject = monsterOn(MK_RAT, 4, 0, FLOOR);
    otherMonster = monsterOn(MK_RAT, 3, 0, FLOOR);
    makeMonsterFollow(subject, leaderMonster);
    makeMonsterFollow(otherMonster, otherLeaderMonster);
}

static boolean monsters_without_a_shared_leader_are_not_teammates(void) {
    TEST_ASSERT(!monstersAreAlliesOf(neutralOne, neutralTwo),
                "two unaligned monsters should not be teammates");
    TEST_ASSERT(!monstersAreAlliesOf(subject, otherMonster),
                "followers of different leaders should not be teammates");
    TEST_ASSERT(!monstersAreAlliesOf(&player, neutralOne),
                "an unaligned monster should not be your teammate");
    return !testFailed;
}

// ---------------------------------------------
// BR89577 — an eel treats a creature in deep water as an enemy
// ---------------------------------------------

static void eel_and_water_creature_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_EEL, 5, 0, DEEP_WATER);
    otherMonster = monsterOn(MK_RAT, 6, 0, DEEP_WATER);
}

static boolean eel_attacks_creature_in_deep_water(void) {
    TEST_ASSERT(monstersAreFoesOf(subject, otherMonster),
                "an eel should treat a creature in deep water as an enemy");
    return !testFailed;
}

static void eel_and_levitating_creature_setup(void) {
    eel_and_water_creature_setup();
    applyStatusToMonster(otherMonster, STATUS_LEVITATING, 100);
}

static boolean eel_ignores_levitating_creature(void) {
    TEST_ASSERT(!monstersAreFoesOf(subject, otherMonster),
                "an eel should not treat a levitating creature as an enemy");
    return !testFailed;
}

// ---------------------------------------------
// BR11561 — monsters never step onto the stairs
// ---------------------------------------------

static void stairs_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    markedPos = posOffset(player.loc, 5, 0);
    placeDownStairs(markedPos);
    subject = monsterOn(MK_RAT, 6, 0, FLOOR);
}

static boolean monster_avoids_stairs(void) {
    TEST_ASSERT(monsterAvoidsCell(subject, markedPos),
                "a monster should avoid the stairs");
    return !testFailed;
}

static boolean player_ignores_stairs(void) {
    TEST_ASSERT(!monsterAvoidsCell(&player, markedPos),
                "you should not avoid the stairs");
    return !testFailed;
}

// ---------------------------------------------
// BR15087 — a liquid creature never moves onto dry land
// ---------------------------------------------

static void liquid_and_dry_land_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_EEL, 5, 0, DEEP_WATER);
    markedPos = posOffset(player.loc, 6, 0);
    setTerrain(markedPos, FLOOR);
}

static boolean liquid_monster_avoids_dry_land(void) {
    TEST_ASSERT(monsterAvoidsCell(subject, markedPos),
                "a liquid creature should avoid dry land");
    return !testFailed;
}

static void liquid_and_water_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_EEL, 5, 0, DEEP_WATER);
    markedPos = posOffset(player.loc, 6, 0);
    setTerrain(markedPos, DEEP_WATER);
}

static boolean liquid_monster_accepts_water(void) {
    TEST_ASSERT(!monsterAvoidsCell(subject, markedPos),
                "a liquid creature should accept deep water");
    return !testFailed;
}

// ---------------------------------------------
// BR50228 — a non-allied monster never avoids your square
// ---------------------------------------------

static void avoid_player_setup(void) {
    clearMonsters();
    // Move off the starting stairs so the stairs rule does not mask this one.
    pos p = posOffset(player.loc, 3, 0);
    setTerrain(p, FLOOR);
    setPlayerPos(p);
    subject = monsterOn(MK_RAT, 6, 0, FLOOR);
}

static boolean hostile_monster_does_not_avoid_player(void) {
    TEST_ASSERT(!monsterAvoidsCell(subject, player.loc),
                "a non-allied monster should not avoid your square");
    return !testFailed;
}

// ---------------------------------------------
// BR27232 — a monster avoids harmful terrain unless immune
// ---------------------------------------------

static void lava_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    markedPos = posOffset(player.loc, 5, 0);
    setTerrain(markedPos, LAVA);
    subject = monsterOn(MK_RAT, 6, 0, FLOOR);
}

static boolean monster_avoids_lava(void) {
    TEST_ASSERT(monsterAvoidsCell(subject, markedPos),
                "a monster should avoid lava");
    return !testFailed;
}

static void fire_immune_lava_setup(void) {
    lava_setup();
    applyStatusToMonster(subject, STATUS_IMMUNE_TO_FIRE, 100);
}

static boolean fire_immune_monster_ignores_lava(void) {
    TEST_ASSERT(!monsterAvoidsCell(subject, markedPos),
                "a fire-immune monster should not avoid lava");
    return !testFailed;
}

// ---------------------------------------------
// BR52904 — a burning wandering monster avoids any flammable ground
//
// A wisp carries an aura of flame.  While wandering it must keep off
// grass, which would catch fire under it; a monster that is not
// burning, or is not wandering, has no such scruple.
// ---------------------------------------------

static creature *flammableSubject;
static pos flammableGround;

static void burning_wanderer_setup(void) {
    setArenaRadius(4, FLOOR);
    flammableGround = posOffset(getPlayerPos(), 2, 0);
    setTerrain(flammableGround, GRASS);
    flammableSubject = spawnMonster(MK_WILL_O_THE_WISP, posOffset(getPlayerPos(), 3, 0));
    makeMonsterUnaware(flammableSubject);
}

static boolean burning_wanderer_avoids_flammable_ground(void) {
    TEST_ASSERT(monsterIsWandering(flammableSubject),
                "the wisp should be wandering");
    TEST_ASSERT(monsterAvoidsCell(flammableSubject, flammableGround),
                "a burning wandering monster should avoid flammable ground");
    return !testFailed;
}

static void burning_tracker_setup(void) {
    setArenaRadius(4, FLOOR);
    flammableGround = posOffset(getPlayerPos(), 2, 0);
    setTerrain(flammableGround, GRASS);
    flammableSubject = spawnMonster(MK_WILL_O_THE_WISP, posOffset(getPlayerPos(), 3, 0));
    wakeMonster(flammableSubject);
}

static boolean burning_tracker_ignores_flammable_ground(void) {
    TEST_ASSERT(monsterIsTrackingPlayer(flammableSubject),
                "the wisp should be tracking you");
    TEST_ASSERT(!monsterAvoidsCell(flammableSubject, flammableGround),
                "a burning monster that is not wandering need not avoid flammable ground");
    return !testFailed;
}

static void nonburning_wanderer_setup(void) {
    setArenaRadius(4, FLOOR);
    flammableGround = posOffset(getPlayerPos(), 2, 0);
    setTerrain(flammableGround, GRASS);
    flammableSubject = spawnMonster(MK_RAT, posOffset(getPlayerPos(), 3, 0));
    makeMonsterUnaware(flammableSubject);
}

static boolean nonburning_wanderer_ignores_flammable_ground(void) {
    TEST_ASSERT(monsterIsWandering(flammableSubject),
                "the rat should be wandering");
    TEST_ASSERT(!monsterAvoidsCell(flammableSubject, flammableGround),
                "a wandering monster that is not burning need not avoid flammable ground");
    return !testFailed;
}

// ---------------------------------------------
// BR51199 — a liquid creature on dry land writhes helplessly: it cannot
// move or attack, and it releases anything it had seized.
// ---------------------------------------------

static boolean beached_seized_player;
static boolean beached_player_escaped;

static void beached_liquid_monster_setup(void) {
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(6, FLOOR);
    // A bog monster lives in liquid and can seize with its attack, so it can
    // exercise all three clauses of the rule: it holds you while it is in
    // water, then is beached in place.
    subject = monsterOn(MK_BOG_MONSTER, 1, 0, DEEP_WATER);
    makeMonsterHostile(subject);
    monsterAttacksPlayer(subject);
    beached_seized_player = playerIsSeized();
    // Beach it where it stands.  setTerrain clears the HAS_MONSTER flag, so
    // put it back on the now-dry tile.
    pos p = monsterPosition(subject);
    setTerrain(p, FLOOR);
    setMonsterPos(subject, p);
    markedPos = p;
    markedHealth = playerHealth();
    monsterTakesTurn(subject);
    // A creature that still held you would cancel this move; a released one
    // lets you step away and clears the seized state.
    pos playerBeforeMove = getPlayerPos();
    moveDirection(LEFT);
    beached_player_escaped = !playerIsAt(playerBeforeMove) && !playerIsSeized();
}

static boolean beached_liquid_monster_cannot_move(void) {
    TEST_ASSERT(beached_seized_player,
                "control: the liquid creature should seize you while it is in water");
    TEST_ASSERT_POS_EQ(monsterPosition(subject), markedPos,
                       "a beached liquid creature should not move");
    TEST_ASSERT_EQ(playerHealth(), markedHealth,
                   "a beached liquid creature should not attack");
    TEST_ASSERT(beached_player_escaped,
                "a beached liquid creature cannot keep hold of a creature it seized");
    return !testFailed;
}

// ---------------------------------------------
// BR44948 — a monster that never sleeps always starts awake
// ---------------------------------------------

static void never_sleeping_monster_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_EEL, 5, 0, FLOOR);
}

static boolean never_sleeping_monster_starts_awake(void) {
    TEST_ASSERT(!monsterIsAsleep(subject),
                "a monster that never sleeps should start awake");
    TEST_ASSERT(monsterIsTrackingPlayer(subject),
                "a monster that never sleeps should start hunting");
    return !testFailed;
}

// ---------------------------------------------
// BR67313 — a raised ally is placed on nearby ground free of harmful terrain
// and occupants
// ---------------------------------------------

static void raised_ally_safe_ground_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);

    // A dead ally awaits resurrection in purgatory; where it died is irrelevant.
    pos ratLoc = posOffset(getPlayerPos(), -6, 0);
    setTerrain(ratLoc, FLOOR);
    addDeadAllyToPurgatory(MK_RAT, ratLoc);

    // The raise point is good, unoccupied ground.
    pos raiseLoc = posOffset(getPlayerPos(), 3, 0);
    setTerrain(raiseLoc, FLOOR);

    resurrectDeadAllyAt(raiseLoc);
}

static boolean raised_ally_placed_on_safe_ground(void) {
    creature *ally = findMonster(MK_RAT);
    TEST_ASSERT(ally != NULL, "an ally should have been raised");
    if (ally == NULL) {
        return !testFailed;
    }

    pos raiseLoc = posOffset(getPlayerPos(), 3, 0);
    TEST_ASSERT_POS_EQ(monsterPosition(ally), raiseLoc,
                       "a raised ally should be placed on the nearby ground");
    TEST_ASSERT(cellIsTerrain(monsterPosition(ally), FLOOR),
                "a raised ally should stand on ordinary ground");
    TEST_ASSERT(!cellIsTerrain(monsterPosition(ally), LAVA),
                "a raised ally should not stand on harmful terrain");
    TEST_ASSERT(monsterIsAlly(ally), "the raised creature should be an ally");
    TEST_ASSERT(monsterIsAlive(ally), "the raised ally should be alive");
    return !testFailed;
}

static void raised_ally_avoids_harmful_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);

    // A dead ally awaits resurrection in purgatory.
    pos ratLoc = posOffset(getPlayerPos(), -6, 0);
    setTerrain(ratLoc, FLOOR);
    addDeadAllyToPurgatory(MK_RAT, ratLoc);

    // The raise point is occupied by the player, and every adjacent square is
    // harmful lava except one safe square to the east.
    pos playerLoc = getPlayerPos();
    setTerrain(posOffset(playerLoc, -1, -1), LAVA);
    setTerrain(posOffset(playerLoc, 0, -1), LAVA);
    setTerrain(posOffset(playerLoc, 1, -1), LAVA);
    setTerrain(posOffset(playerLoc, -1, 0), LAVA);
    setTerrain(posOffset(playerLoc, 1, 0), FLOOR);
    setTerrain(posOffset(playerLoc, -1, 1), LAVA);
    setTerrain(posOffset(playerLoc, 0, 1), LAVA);
    setTerrain(posOffset(playerLoc, 1, 1), LAVA);

    resurrectDeadAllyAt(playerLoc);
}

static boolean raised_ally_avoids_harmful_terrain_and_occupants(void) {
    creature *ally = findMonster(MK_RAT);
    TEST_ASSERT(ally != NULL, "an ally should have been raised");
    if (ally == NULL) {
        return !testFailed;
    }

    pos safeSquare = posOffset(getPlayerPos(), 1, 0);
    TEST_ASSERT_POS_EQ(monsterPosition(ally), safeSquare,
                       "a raised ally should avoid harmful terrain and occupants");
    TEST_ASSERT(cellIsTerrain(monsterPosition(ally), FLOOR),
                "a raised ally should stand on ordinary ground");
    TEST_ASSERT(!cellIsTerrain(monsterPosition(ally), LAVA),
                "a raised ally should not stand on harmful terrain");
    TEST_ASSERT(monsterIsAlly(ally), "the raised creature should be an ally");
    TEST_ASSERT(monsterIsAlive(ally), "the raised ally should be alive");
    return !testFailed;
}

// ============================================================
// BR44825 — an attack is futile against a target that is invulnerable (and
// likewise embedded in walls, weapon-immune to a non-poisonous attacker, or
// levitating while the attacker is water-bound); a monster will not engage
// such a target
// ============================================================

static creature *futilitySubject;
static pos futilityTarget;

static void futile_invulnerable_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    futilitySubject = monsterOn(MK_RAT, 1, 0, FLOOR);
    makeMonsterAlly(futilitySubject);
    futilityTarget = posOffset(monsterPosition(futilitySubject), 1, 0);
    setTerrain(futilityTarget, FLOOR);
    creature *warden = spawnMonster(MK_WARDEN_OF_YENDOR, futilityTarget);
    makeMonsterHostile(warden);
}

static void futile_normal_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    futilitySubject = monsterOn(MK_RAT, 1, 0, FLOOR);
    makeMonsterAlly(futilitySubject);
    futilityTarget = posOffset(monsterPosition(futilitySubject), 1, 0);
    setTerrain(futilityTarget, FLOOR);
    creature *rat = spawnMonster(MK_RAT, futilityTarget);
    makeMonsterHostile(rat);
}

static boolean monster_avoids_invulnerable_enemy(void) {
    TEST_ASSERT(monsterAvoidsCell(futilitySubject, futilityTarget),
                "a monster should avoid attacking an invulnerable enemy, since the attack is futile");
    return !testFailed;
}

static boolean monster_does_not_avoid_normal_enemy(void) {
    TEST_ASSERT(!monsterAvoidsCell(futilitySubject, futilityTarget),
                "a monster should not avoid attacking a normal, damageable enemy");
    return !testFailed;
}

// ============================================================
// BR83744 — a monster will not cast at a target that is hidden
// from it, submerged, or without a clear line of fire
//
// The caster is a mirrored totem: it never moves, never stops hunting, and
// always uses its beckoning spell when a target is eligible, so whether the
// target is pulled toward it shows exactly when the cast was allowed.
// ============================================================

static boolean samePos(pos a, pos b) {
    return a.x == b.x && a.y == b.y;
}

// --- hidden target ------------------------------------------------------

static creature *hiddenCaster;
static pos hiddenStart;
static boolean castAtVisibleTarget, castAtHiddenTarget, castAtRevealedTarget;

static void hidden_target_setup(void) {
    setArenaRadius(6, FLOOR);
    hiddenStart = getPlayerPos();
    hiddenCaster = placeHostileMonster(MK_MIRRORED_TOTEM, posOffset(hiddenStart, -4, 0));
}

static boolean hidden_target_step(void) {
    monsterTakesTurn(hiddenCaster);
    castAtVisibleTarget = !playerIsAt(hiddenStart);
    setPlayerPos(hiddenStart);

    applyStatusToPlayer(STATUS_INVISIBLE, 100);
    monsterTakesTurn(hiddenCaster);
    castAtHiddenTarget = !playerIsAt(hiddenStart);
    setPlayerPos(hiddenStart);

    removeStatusFromPlayer(STATUS_INVISIBLE);
    monsterTakesTurn(hiddenCaster);
    castAtRevealedTarget = !playerIsAt(hiddenStart);
    setPlayerPos(hiddenStart);
    return false;
}

static boolean monster_will_not_cast_at_hidden_target(void) {
    TEST_ASSERT(castAtVisibleTarget,
                "control: the totem should cast at a target it can see");
    TEST_ASSERT(!castAtHiddenTarget,
                "a monster should not cast at a target that is hidden from it");
    TEST_ASSERT(castAtRevealedTarget,
                "control: the totem should cast again once the target is visible");
    return !testFailed;
}

// --- no clear line of fire ----------------------------------------------

static creature *walledCaster;
static pos walledStart;
static boolean castWithClearLine, castWithWallBetween, castAfterWallRemoved;

static void line_of_fire_setup(void) {
    setArenaRadius(6, FLOOR);
    walledStart = getPlayerPos();
    walledCaster = placeHostileMonster(MK_MIRRORED_TOTEM, posOffset(walledStart, -4, 0));
}

static void setWallBetweenCasterAndPlayer(boolean wallPresent) {
    short tile = wallPresent ? GRANITE : FLOOR;
    for (int dy = -6; dy <= 6; dy++) {
        setTerrain(posOffset(walledStart, -2, dy), tile);
    }
}

static boolean line_of_fire_step(void) {
    monsterTakesTurn(walledCaster);
    castWithClearLine = !playerIsAt(walledStart);
    setPlayerPos(walledStart);

    setWallBetweenCasterAndPlayer(true);
    monsterTakesTurn(walledCaster);
    castWithWallBetween = !playerIsAt(walledStart);
    setPlayerPos(walledStart);

    setWallBetweenCasterAndPlayer(false);
    monsterTakesTurn(walledCaster);
    castAfterWallRemoved = !playerIsAt(walledStart);
    setPlayerPos(walledStart);
    return false;
}

static boolean monster_will_not_cast_without_line_of_fire(void) {
    TEST_ASSERT(castWithClearLine,
                "control: the totem should cast with a clear line of fire");
    TEST_ASSERT(!castWithWallBetween,
                "a monster should not cast at a target without a clear line of fire");
    TEST_ASSERT(castAfterWallRemoved,
                "control: the totem should cast again once the line of fire is clear");
    return !testFailed;
}

// --- submerged target ----------------------------------------------------

static creature *submergedCaster, *submergedAlly;
static pos submergedStart, submergedAllyStart;
static boolean castAtVisibleAlly, noCastAtSubmergedAlly, castAtResurfacedAlly;

static void submerged_target_setup(void) {
    setArenaRadius(6, FLOOR);
    submergedStart = getPlayerPos();
    submergedCaster = placeHostileMonster(MK_MIRRORED_TOTEM, posOffset(submergedStart, -4, 0));

    // Stay hidden from the totem yourself, so its only possible target is
    // the ally.
    applyStatusToPlayer(STATUS_INVISIBLE, 10000);

    submergedAllyStart = posOffset(submergedStart, 0, -3);
    setTerrain(submergedAllyStart, FLOOR);
    submergedAlly = spawnMonster(MK_RAT, submergedAllyStart);
    makeMonsterAlly(submergedAlly);
    revealCell(submergedAllyStart);
}

static boolean submerged_target_step(void) {
    monsterTakesTurn(submergedCaster);
    castAtVisibleAlly = !samePos(monsterPosition(submergedAlly), submergedAllyStart);
    setMonsterPos(submergedAlly, submergedAllyStart);

    setMonsterSubmerged(submergedAlly, true);
    monsterTakesTurn(submergedCaster);
    noCastAtSubmergedAlly = samePos(monsterPosition(submergedAlly), submergedAllyStart);
    setMonsterPos(submergedAlly, submergedAllyStart);

    setMonsterSubmerged(submergedAlly, false);
    monsterTakesTurn(submergedCaster);
    castAtResurfacedAlly = !samePos(monsterPosition(submergedAlly), submergedAllyStart);
    setMonsterPos(submergedAlly, submergedAllyStart);
    return false;
}

static boolean monster_will_not_cast_at_submerged_target(void) {
    TEST_ASSERT(castAtVisibleAlly,
                "control: the totem should cast at an ally it can see");
    TEST_ASSERT(noCastAtSubmergedAlly,
                "a monster should not cast at a target that is submerged");
    TEST_ASSERT(castAtResurfacedAlly,
                "control: the totem should cast again once the target surfaces");
    return !testFailed;
}

// ---------------------------------------------
// BR83001 — a monster wears down the webbing it is caught in, escaping after
// a few turns; a monster that cannot be harmed by webs walks free at once
// ---------------------------------------------

static creature *webbedSubject;
static pos webbedStart;
static int struggleTurns;

// Open floor for four squares in every direction, with a web three squares to
// the player's right.  Returns a monster standing in that web, hunting you.
static creature *monsterStandingInWeb(enum monsterTypes type) {
    setArenaRadius(4, FLOOR);
    pos webLoc = posOffset(player.loc, 3, 0);
    setTerrain(webLoc, SPIDERWEB);
    creature *caught = spawnMonster(type, webLoc);
    makeMonsterHostile(caught);
    return caught;
}

static void webbed_monster_setup(void) {
    webbedSubject = monsterStandingInWeb(MK_RAT);
    // Three turns of struggling, as webs normally give a creature.
    applyStatusToMonster(webbedSubject, STATUS_STUCK, 3);
    webbedStart = monsterPosition(webbedSubject);
    struggleTurns = 0;
}

// Take turns for the monster until it manages to walk off the web.
static boolean webbed_monster_struggles_step(void) {
    monsterTakesTurn(webbedSubject);
    struggleTurns++;
    pos here = monsterPosition(webbedSubject);
    return here.x == webbedStart.x && here.y == webbedStart.y && struggleTurns < 20;
}

static boolean webbed_monster_escapes_after_a_few_turns(void) {
    pos here = monsterPosition(webbedSubject);
    TEST_ASSERT(here.x != webbedStart.x || here.y != webbedStart.y,
                "a monster caught in a web should struggle free and walk away");
    TEST_ASSERT(struggleTurns >= 2,
                "a monster caught in a web should need several turns to escape");
    TEST_ASSERT_EQ(monsterStatusTurns(webbedSubject, STATUS_STUCK), 0,
                   "an escaped monster should no longer be entangled");
    return !testFailed;
}

static creature *webImmuneSubject;
static pos webImmuneStart;
static int freeTurns;

static void web_immune_monster_setup(void) {
    webImmuneSubject = monsterStandingInWeb(MK_PHANTOM);
    // Webs cannot hold a creature that is immune to them, so the entangling
    // status is applied anyway and must be ignored.
    applyStatusToMonster(webImmuneSubject, STATUS_STUCK, 3);
    webImmuneStart = monsterPosition(webImmuneSubject);
    freeTurns = 0;
}

// Take a single turn for the monster; keep going only if it failed to leave.
static boolean web_immune_monster_turn_step(void) {
    monsterTakesTurn(webImmuneSubject);
    freeTurns++;
    pos here = monsterPosition(webImmuneSubject);
    return here.x == webImmuneStart.x && here.y == webImmuneStart.y && freeTurns < 10;
}

static boolean web_immune_monster_walks_free_immediately(void) {
    pos here = monsterPosition(webImmuneSubject);
    TEST_ASSERT(here.x != webImmuneStart.x || here.y != webImmuneStart.y,
                "a monster immune to webs should not be held by a web");
    TEST_ASSERT_EQ(freeTurns, 1,
                   "a monster immune to webs should walk out of the web in a single turn");
    return !testFailed;
}

// ============================================================
// BR39283 — some hordes are summoned at a distance: their members
// appear outside your field of view, within half the level's width in
// travel distance, and never in harmful terrain or on occupied squares
// ============================================================

static creature *distanceSummoner;
static int summoningTurns;

// The goblin warlord's summon horde is the one horde flagged to appear at a
// distance.  Summoning it as an ally lets it cast on its own turn without
// first having to notice you, and no other monster shares the level with it,
// so every monster but the warlord is a summoned member.
static void summoned_at_distance_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    distanceSummoner = monsterOn(MK_GOBLIN_CHIEFTAN, 3, 0, FLOOR);
    makeMonsterAlly(distanceSummoner);
    summoningTurns = 0;
}

// Give the warlord turns until its summoning spell lands; while it has fewer
// than five followers it has a one-in-five chance each turn.
static boolean summoned_at_distance_step(void) {
    if (monsterCount() > 1) {
        return false;
    }
    summoningTurns++;
    if (summoningTurns > 100) {
        return false;
    }
    monsterTakesTurn(distanceSummoner);
    return true;
}

static boolean summoned_members_appear_at_distance(void) {
    TEST_ASSERT(monsterCount() > 1,
                "the warlord should summon its conjurer and goblins before the turns run out");

    pos summonerPos = monsterPosition(distanceSummoner);
    int memberCount = 0;

    for (int i = 0; i < monsterCount(); i++) {
        creature *member = monsterAtIndex(i);
        if (member == NULL || member == distanceSummoner) {
            continue;
        }
        memberCount++;
        pos memberPos = monsterPosition(member);

        TEST_ASSERT(!cellIsVisible(memberPos),
                    "summoned members should appear outside your field of view");

        int travel = pathingDistanceBetween(summonerPos, memberPos, 0);
        TEST_ASSERT(travel >= 1 && travel <= levelWidth() / 2,
                    "summoned members should appear within half the level's width in travel distance of their summoner");

        TEST_ASSERT(!cellIsHarmfulTerrain(memberPos),
                    "summoned members should never appear in harmful terrain");

        TEST_ASSERT(!playerIsAt(memberPos)
                        && (memberPos.x != summonerPos.x || memberPos.y != summonerPos.y),
                    "summoned members should never appear on an occupied square");

        for (int j = i + 1; j < monsterCount(); j++) {
            creature *other = monsterAtIndex(j);
            if (other == NULL || other == distanceSummoner) {
                continue;
            }
            pos otherPos = monsterPosition(other);
            TEST_ASSERT(memberPos.x != otherPos.x || memberPos.y != otherPos.y,
                        "no two summoned members should appear on the same square");
        }
    }
    TEST_ASSERT(memberCount > 0,
                "the summoned members should be inspectable one by one");
    return !testFailed;
}

// ---------------------------------------------
// BR51745 — A monster may step onto an adjacent unfriendly monster it is
// willing to fight, but avoids that square if the attack would be futile.
//
// The observable is whether the monster avoids the target's square.  The
// subject is your ally, so it is willing to fight a hostile neighbour; the
// positive and negative cases differ only in whether that neighbour can
// actually be harmed.
// ---------------------------------------------

static creature *adjSubject;
static pos adjTarget;

// Lay a patch of open floor, put an allied subject monster on it, and set a
// hostile target of the given kind on the square right next to the subject.
static void adjacent_scene(enum monsterTypes targetType) {
    pos base = posAt(40, 14);
    setTerrain(base, FLOOR);
    setPlayerPos(base);
    setArenaRadius(4, FLOOR);          // clears monsters and squares off floor

    adjTarget = posOffset(base, 3, 0);
    pos subjPos = posOffset(base, 2, 0);
    adjSubject = spawnMonster(MK_RAT, subjPos);
    makeMonsterAlly(adjSubject);
    creature *target = spawnMonster(targetType, adjTarget);
    makeMonsterHostile(target);
}

static void fightable_adjacent_setup(void) {
    adjacent_scene(MK_RAT);             // a damageable hostile neighbour
}

static boolean monster_steps_onto_fightable_adjacent(void) {
    TEST_ASSERT(!monsterAvoidsCell(adjSubject, adjTarget),
                "a monster should not avoid the square of an adjacent enemy it can fight");
    return !testFailed;
}

static void futile_adjacent_setup(void) {
    adjacent_scene(MK_WARDEN_OF_YENDOR); // invulnerable, so the attack is futile
}

static boolean monster_avoids_futile_adjacent(void) {
    TEST_ASSERT(monsterAvoidsCell(adjSubject, adjTarget),
                "a monster should avoid the square of an adjacent enemy it cannot harm");
    return !testFailed;
}

// ============================================================
// BR52591 — a monster moving toward a destination tries the diagonal
// step first, then falls back to the two cardinal directions, and then
// to off-axis steps.
// ============================================================
//
// An allied rat two squares due east of you walks toward a hostile rat
// two squares due north of you: the target lies two squares diagonally
// away, with equal east-west and north-south gaps, so every attempt the
// walking monster makes is fully determined.  Each variant walls off or
// poisons one more tier of the step order and asserts which square the
// monster actually steps into.

static creature *br52591_ally;
static pos br52591_playerPos;

// Lay the scene: open floor around the player, a hostile rat two squares
// due north, and an allied rat two squares due east that will walk north-
// west toward it on its next turn.
static void br52591_scene(void) {
    clearArena();
    br52591_playerPos = getPlayerPos();
    placeHostileMonster(MK_RAT, posOffset(br52591_playerPos, 0, -2));
    br52591_ally = spawnMonster(MK_RAT, posOffset(br52591_playerPos, 2, 0));
    makeMonsterAlly(br52591_ally);
}

// Give the walking monster its one turn.
static boolean br52591_take_turn_step(void) {
    monsterTakesTurn(br52591_ally);
    return false;
}

// --- open ground: the diagonal step comes first -------------------

static void br52591_diagonal_setup(void) {
    br52591_scene();
}

static boolean br52591_diagonal_step_taken(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(br52591_ally),
                       posOffset(br52591_playerPos, 1, -1),
                       "with the diagonal square open, the monster should take the diagonal step first");
    return !testFailed;
}

// --- diagonal step refused: fall back to a cardinal step -----------

static void br52591_cardinal_setup(void) {
    br52591_scene();
    // Wall one flank of the diagonal step (the square south of the
    // diagonal destination), which closes off the diagonal way while
    // leaving the straight line to the target and both cardinal
    // squares open.
    setTerrain(posOffset(br52591_playerPos, 2, -1), GRANITE);
}

static boolean br52591_cardinal_step_taken(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(br52591_ally),
                       posOffset(br52591_playerPos, 1, 0),
                       "with the diagonal way walled off, the monster should fall back to a cardinal step");
    return !testFailed;
}

// --- diagonal refused and both cardinal squares refused: off-axis --

static void br52591_off_axis_setup(void) {
    br52591_scene();
    // The diagonal way is walled off by the granite flank; the first
    // cardinal square holds lava, which the monster will not enter, and
    // the second cardinal square is the granite wall itself.  Only the
    // off-axis steps remain.
    setTerrain(posOffset(br52591_playerPos, 2, -1), GRANITE);
    setTerrain(posOffset(br52591_playerPos, 1, 0), LAVA);
}

static boolean br52591_off_axis_step_taken(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(br52591_ally),
                       posOffset(br52591_playerPos, 1, 1),
                       "with the diagonal way and both cardinal squares refused, the monster should fall back to an off-axis step");
    return !testFailed;
}

// ============================================================
// BR53265 — a wandering monster travels between checkpoints,
// choosing ones it has not recently visited and clearing roughly
// half of them at a time.
// ============================================================
//
// A monster set to wander is left alone on the generated level, far
// from you, and given many turns.  Travelling between checkpoints shows
// up as long trips away from its starting square throughout the run: it
// gets far from where it began, it is still reaching distant parts of
// the level late in the run — when every nearby checkpoint has already
// been visited — and it keeps moving instead of settling on one spot.

#define BR53265_TURNS 150

static creature *br53265_subject;
static pos br53265_start;
static int br53265_turns;
static int br53265_maxDist;       // farthest square reached from the start
static int br53265_lateMaxDist;   // farthest reached over the last half
static int br53265_startGap;      // how far from the player it started
static int br53265_lateMoves;     // position changes over the last 30 turns
static boolean br53265_stayedWandering;

static int br53265_dist(pos a, pos b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

static void br53265_setup(void) {
    clearMonsters();
    pos playerPos = getPlayerPos();
    // Wall the player into their starting square, so no checkpoint route
    // can ever bring the wanderer into sight of them: it must keep to its
    // checkpoints for the whole run instead of noticing you and tracking.
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx != 0 || dy != 0) {
                setTerrain(posOffset(playerPos, dx, dy), GRANITE);
            }
        }
    }
    // Take one player turn so field of view is recomputed against the new
    // walls; without it the mask from level generation still covers the
    // squares around the player and the wanderer could notice them.
    restOneTurn();
    // Spawn the wanderer on the floor square farthest from the player,
    // so it starts a long way from the walled-in player.
    pos best = playerPos;
    int bestDist = -1;
    for (int x = 1; x < levelWidth() - 1; x++) {
        for (int y = 1; y < levelHeight() - 1; y++) {
            pos candidate = posAt(x, y);
            if (terrainAt(candidate) == FLOOR) {
                int d = br53265_dist(candidate, playerPos);
                if (d > bestDist) {
                    bestDist = d;
                    best = candidate;
                }
            }
        }
    }
    br53265_startGap = bestDist;
    br53265_subject = spawnMonster(MK_RAT, best);
    makeMonsterUnaware(br53265_subject);
    br53265_start = best;
    br53265_turns = 0;
    br53265_maxDist = 0;
    br53265_lateMaxDist = 0;
    br53265_lateMoves = 0;
    br53265_stayedWandering = true;
}

static boolean br53265_step(void) {
    if (br53265_turns >= BR53265_TURNS) {
        return false;
    }
    pos before = monsterPosition(br53265_subject);
    monsterTakesTurn(br53265_subject);
    pos here = monsterPosition(br53265_subject);
    br53265_turns++;

    if (!monsterIsWandering(br53265_subject)) {
        br53265_stayedWandering = false;
        return false; // setup broken: it noticed you
    }

    int d = br53265_dist(here, br53265_start);
    if (d > br53265_maxDist) {
        br53265_maxDist = d;
    }
    if (br53265_turns > BR53265_TURNS / 2 && d > br53265_lateMaxDist) {
        br53265_lateMaxDist = d;
    }
    if (br53265_turns > BR53265_TURNS - 30
        && (here.x != before.x || here.y != before.y)) {
        br53265_lateMoves++;
    }
    return true;
}

static boolean wanders_between_distant_checkpoints(void) {
    TEST_ASSERT(br53265_stayedWandering,
                "setup: the wanderer must never notice you while it travels its checkpoints");
    TEST_ASSERT(br53265_startGap >= 15,
                "setup: the wanderer should start a long way from the player");
    TEST_ASSERT(br53265_maxDist >= 15,
                "a wandering monster should travel far from where it began, out to checkpoints across the level");
    TEST_ASSERT(br53265_lateMaxDist >= 12,
                "late in the run, with every nearby checkpoint already visited, it should still be heading out to distant parts of the level");
    TEST_ASSERT(br53265_lateMoves >= 15,
                "with roughly half its checkpoints always available again, it should still be travelling in the last stretch of the run instead of settling on one spot");
    return !testFailed;
}

// ---------------------------------------------
// BR66644 — an ally within 3 squares of you that can see you
// wanders idly about 30% of the time instead of following you
// ---------------------------------------------

static creature *br66644_ally;
static int br66644_movedTurns;

static void br66644_ally_setup(void) {
    // Open floor around the player; this also clears monsters and floor food.
    setArenaRadius(6, FLOOR);
    restOneTurn(); // establish sight over the cleared floor
    pos start = posOffset(getPlayerPos(), 2, 0);
    br66644_ally = spawnMonster(MK_RAT, start);
    makeMonsterAlly(br66644_ally);
    TEST_ASSERT(cellIsVisible(start),
                "setup: the ally must stand within sight of the player");
}

static boolean br66644_ally_step(void) {
    // Give the ally 200 turns and count how often it actually moves.  With no
    // enemies around, a close visible ally should take a step on only about
    // 30% of its turns; blindly following you every turn would move far more
    // often, and never moving at all far less.
    br66644_movedTurns = 0;
    pos prev = monsterPosition(br66644_ally);
    for (int i = 0; i < 200; i++) {
        monsterTakesTurn(br66644_ally);
        pos now = monsterPosition(br66644_ally);
        if (!posEq(now, prev)) {
            br66644_movedTurns++;
        }
        prev = now;
    }
    return false;
}

static boolean ally_idles_when_close_and_seen(void) {
    TEST_ASSERT(br66644_movedTurns >= 25 && br66644_movedTurns <= 130,
                "an ally that can see you up close should move on only a minority of its turns (about 30%)");
    return !testFailed;
}

// ---------------------------------------------
// BR54211 — a permanently fleeing thief that loses the item it stole
// returns to normal behaviour: an ally if it follows you, otherwise hunting
// ---------------------------------------------

static creature *br54211_thief;

// Open floor, with a monkey that has just stolen a ration and fled with it.
static void br54211_thief_setup(void) {
    setArenaRadius(4, FLOOR);
    br54211_thief = placeHostileMonster(MK_MONKEY, posOffset(getPlayerPos(), 4, 0));
    makeMonsterStealAndFlee(br54211_thief, FOOD, RATION);
    TEST_ASSERT(monsterHasAbility(br54211_thief, MA_HIT_STEAL_FLEE),
                "setup: the monkey should be a steal-and-flee monster");
    TEST_ASSERT(monsterIsCarryingItem(br54211_thief),
                "setup: the thief should be holding the stolen item");
    TEST_ASSERT(monsterIsPermFleeing(br54211_thief),
                "setup: the thief should be in permanent flight");
    TEST_ASSERT(monsterIsFleeing(br54211_thief),
                "setup: the thief should be fleeing");
}

static boolean br54211_hunting_step(void) {
    monsterLosesCarriedItem(br54211_thief);
    TEST_ASSERT(!monsterIsCarryingItem(br54211_thief),
                "setup: the thief should now be empty-handed");
    // A full turn: the player acts (refreshing the scent trail), then the
    // thief takes the turn in which it gives up its flight.
    restOneTurn();
    return false;
}

static boolean br54211_returns_to_hunting(void) {
    TEST_ASSERT(monsterIsAlive(br54211_thief), "control: the thief survives");
    TEST_ASSERT(!monsterIsPermFleeing(br54211_thief),
                "losing the stolen item should end permanent flight");
    TEST_ASSERT(!monsterIsFleeing(br54211_thief),
                "the thief should stop fleeing");
    TEST_ASSERT(monsterIsTrackingPlayer(br54211_thief),
                "a thief that does not follow you should go back to hunting you");
    return !testFailed;
}

// The same thief, except that it follows you: it must come back as an ally.
static void br54211_follower_setup(void) {
    br54211_thief_setup();
    makeMonsterFollowPlayer(br54211_thief);
    TEST_ASSERT(monsterIsAlive(br54211_thief), "setup: the following thief should be alive");
    TEST_ASSERT(!monsterIsAlly(br54211_thief),
                "setup: it should still be a fleeing thief until it loses the item");
}

static boolean br54211_ally_step(void) {
    monsterLosesCarriedItem(br54211_thief);
    restOneTurn();
    return false;
}

static boolean br54211_returns_to_ally(void) {
    TEST_ASSERT(!monsterIsPermFleeing(br54211_thief),
                "losing the stolen item should end permanent flight");
    TEST_ASSERT(monsterIsAlly(br54211_thief),
                "a thief that follows you and loses the item should become an ally");
    return !testFailed;
}

// ============================================================
// BR46360 — a level begins populated with 6 monsters, plus 3
// more for each depth below the amulet depth, up to a maximum
// of 20.
//
// Three first-time descents take the census at a shallow depth
// (base 6), two depths below the amulet depth (6 + 3 + 3 = 12),
// and depth 39, the deepest level any horde will spawn on (where
// the uncapped formula would ask for 6 + 3 * 13 = 45).  The
// lower bounds falsify a flat or lagging population; the upper
// bound on the depth-39 census is what falsifies the missing
// cap — the capped run seeds ~20 hordes where an uncapped run
// would seed ~45, and the census tracks that difference closely
// on the fixed test seed (71 observed), so 100 separates the two
// with room to spare while staying far below the ~150 an
// uncapped population reaches.  The fixed seed makes every
// census reproducible.
// ============================================================

#define BR46360_DEEP_CENSUS_LIMIT 100

static boolean population_matches_the_depth_formula(void) {
    while (currentDepth() < 2) {
        descendToNextLevel();
    }
    int shallow = monsterCount();

    setDepth(27);
    descendToNextLevel();
    int belowAmulet = monsterCount();

    setDepth(38);
    descendToNextLevel();
    int deep = monsterCount();

    TEST_ASSERT(shallow >= 6,
                "a level at the surface begins with the base of 6 monsters");
    TEST_ASSERT(belowAmulet >= 12,
                "two depths below the amulet depth adds 3 monsters per depth to the base");
    TEST_ASSERT(deep >= 20,
                "population grows with depth until it reaches the cap of 20");
    TEST_ASSERT(deep <= BR46360_DEEP_CENSUS_LIMIT,
                "the cap keeps a deep level far below the uncapped 45-horde population");
    return !testFailed;
}

// ============================================================
// BR59340 (player half) — allies will not negate a player
// knowingly wearing identified reflection armor.
//
// First a control: with a shielded player and no reflection
// armor, a hostile dar priestess finds a reason to negate and
// must strip the shield, proving this harness can catch a
// negation aimed at the player.  Then the rule's own scenario:
// the same caster becomes your ally and you wear identified
// armor of reflection — for the rest of the run it must never
// cast a negation spell at you and your shield must stand.
// ============================================================

static creature *br59340_priestess;
static pos br59340_playerSpot;
static boolean br59340_hostileCast;
static boolean br59340_rulePhase;
static int br59340_turns;

static void br59340_player_setup(void) {
    setArenaRadius(6, FLOOR);
    br59340_playerSpot = getPlayerPos();
    // Aggravate yourself before the turn below: it pins the engine's stealth
    // range to a huge value when the turn recomputes it, so the caster's
    // awareness of you is deterministic instead of a per-turn dice roll that
    // could drop it out of its hunting state mid-test.
    applyStatusToPlayer(STATUS_AGGRAVATING, 10000);
    restOneTurn(); // one real turn so the engine recomputes the field of view over the cleared arena
    applyStatusToPlayer(STATUS_SHIELDED, 10000);
    br59340_priestess = placeHostileMonster(MK_DAR_PRIESTESS,
                                            posOffset(br59340_playerSpot, -4, 0));
    br59340_hostileCast = false;
    br59340_rulePhase = false;
    br59340_turns = 0;
}

static boolean br59340_player_step(void) {
    monsterTakesTurn(br59340_priestess);
    br59340_turns++;
    setPlayerHealth(playerMaxHealth()); // shrug off the sparks and blows of the control phase

    if (!br59340_rulePhase) {
        if (playerHasStatus(STATUS_SHIELDED) && br59340_turns < 40) {
            return true; // keep rolling until the control negation lands (or time out)
        }
        // Only negation strips a shield, so its disappearance is the control
        // signal (spell announcements depend on lighting and cannot be
        // relied on at every distance).
        br59340_hostileCast = !playerHasStatus(STATUS_SHIELDED);
        // Switch to the rule's scenario: same caster, now your ally, and you
        // are knowingly wearing identified armor of reflection.
        givePlayerRunicArmor(CHAIN_MAIL, A_REFLECTION, 3, true);
        applyStatusToPlayer(STATUS_SHIELDED, 10000);
        makeMonsterAlly(br59340_priestess);
        // If the ally ever broke the rule, its negation would either strip
        // your shield (bolt lands) or, reflected by your armor, strip the
        // ally's own haste (bolt bounces) — one of the two must survive.
        applyStatusToMonster(br59340_priestess, STATUS_HASTED, 10000);
        clearMessages();
        br59340_rulePhase = true;
        br59340_turns = 0;
        return true;
    }
    return br59340_turns < 40;
}

static boolean br59340_player_verify(void) {
    item *armor = equippedArmor();
    TEST_ASSERT(br59340_hostileCast,
                "control: a hostile caster must negate a shielded player");
    TEST_ASSERT(armor != NULL && runicIsIdentified(armor),
                "the player is knowingly wearing identified reflection armor");
    TEST_ASSERT(playerHasStatus(STATUS_SHIELDED),
                "an ally will not negate a player knowingly wearing identified reflection armor");
    TEST_ASSERT(monsterHasStatus(br59340_priestess, STATUS_HASTED),
                "the ally never negated itself through the reflected spell either");
    TEST_ASSERT(!messageWasShown("casts a negation spell"),
                "the ally never casts a negation spell at the player");
    if (testFailed) {
        pos mp = monsterPosition(br59340_priestess);
        pos pp = getPlayerPos();
    }
    return !testFailed;
}

// ============================================================
// BR59340 (creature half) — allies will never destroy an allied
// creature with negation.
//
// Phase one: your allied dar priestess spends 30 turns beside an
// allied phylactery — a creature negation would destroy — with
// the player visible so the caster runs its spell selection
// every turn; the phylactery must live.  Phase two, the control:
// the caster turns hostile, the still-allied phylactery is then
// a legitimate enemy target for it, and the same caster's
// negation must destroy it — without the rule, phase one and
// phase two look the same.  The phylactery (not the golem) is
// the dies-if-negated creature of choice because it never
// reflects: a reflected negation would strike the caster itself,
// stripping its spells and ending the control on a coin flip.
// ============================================================

static creature *br59340b_priestess;
static creature *br59340b_fragile;
static boolean br59340b_rulePhase;
static boolean br59340b_allySafe;
static boolean br59340b_controlKilled;
static int br59340b_turns;

static void br59340b_setup(void) {
    setArenaRadius(14, FLOOR);
    pos spot = getPlayerPos();
    // Aggravate yourself before the turn below: it pins the engine's stealth
    // range to a huge value when the turn recomputes it, so the caster's
    // awareness of you is deterministic instead of a per-turn dice roll that
    // could drop it out of its hunting state mid-test.
    applyStatusToPlayer(STATUS_AGGRAVATING, 10000);
    restOneTurn(); // one real turn so the engine recomputes the field of view over the cleared arena
    // Seal a full-height wall between you and the caster's side of the
    // arena.  The caster still sees you (its tiles were in the field of view
    // computed above, and no player turn recomputes it), so it keeps its
    // hunting state, but you are no longer a reachable bolt target — the
    // phylactery is its only possible target, which keeps every
    // spell-selection roll on the creature the rule is about.  The
    // phylactery sits 14 tiles from you while the caster never roams more
    // than 10 (its leash), so the two are always at least 4 tiles apart:
    // closer than that the caster, which keeps its distance from everything,
    // would settle into a fleeing state where it casts nothing at all.  You
    // are 4+ tiles from the caster's stopping point on the wall for the same
    // reason.
    for (int y = 0; y < levelHeight(); y++) {
        setTerrain(posAt(spot.x - 3, y), GRANITE);
    }
    br59340b_priestess = spawnMonster(MK_DAR_PRIESTESS, posOffset(spot, -6, 0));
    makeMonsterAlly(br59340b_priestess);
    br59340b_fragile = spawnMonster(MK_PHYLACTERY, posOffset(spot, -14, 0));
    makeMonsterAlly(br59340b_fragile);
    // Raise its hit points far above what stray sparks can chip off, so
    // only negation can ever make it dead and the control signal stays
    // pure.
    setMonsterHealth(br59340b_fragile, 1000);
    revealCell(monsterPosition(br59340b_fragile));
    br59340b_rulePhase = true;
    br59340b_allySafe = false;
    br59340b_controlKilled = false;
    br59340b_turns = 0;
}

static boolean br59340b_step(void) {
    monsterTakesTurn(br59340b_priestess);
    br59340b_turns++;

    if (br59340b_rulePhase) {
        if (br59340b_turns >= 30) {
            br59340b_allySafe = monsterIsAlive(br59340b_fragile);
            // Control: with the caster turned against you, the still-allied
            // phylactery is a legitimate enemy target for it, so the same
            // caster's negation must destroy it.
            makeMonsterHostile(br59340b_priestess);
            br59340b_rulePhase = false;
            br59340b_turns = 0;
        }
        return true;
    }
    if (monsterIsDead(br59340b_fragile)) {
        // Nothing but negation can destroy a creature that dies if negated
        // (its hit points are out of reach of every other effect here), so
        // its death is the control signal (spell announcements depend on
        // lighting and cannot be relied on at every distance).
        br59340b_controlKilled = true;
        return false;
    }
    return br59340b_turns < 40;
}

static boolean br59340b_verify(void) {
    TEST_ASSERT(br59340b_allySafe,
                "an ally will not negate — and so never destroy — an allied creature");
    TEST_ASSERT(br59340b_controlKilled,
                "control: the same caster's negation destroys that creature when it is an enemy");
    return !testFailed;
}

// ============================================================
// BR20368 — immobile monsters such as totems are aware of you only
// while you are within their detection range, and otherwise stand
// inert as though asleep
// ============================================================

static creature *br20368_nearTotem;
static creature *br20368_farTotem;

static void br20368_setup(void) {
    clearArena();
    pos playerStart = getPlayerPos();
    // A clear east-west line: one totem two squares away, one twenty, so
    // both sit at unambiguous ends of the detection range.
    int step = (playerStart.x + 20 <= levelWidth() - 2) ? 1 : -1;
    for (int i = 1; i <= 20; i++) {
        setTerrain(posAt(playerStart.x + step * i, playerStart.y), FLOOR);
    }
    // Carving the corridor invalidates the field of view and the scent map
    // computed at level start; a resting turn recomputes both over the new
    // open floor before either totem is placed.
    restOneTurn();
    br20368_nearTotem = placeHostileMonster(MK_GOBLIN_TOTEM,
                                             posAt(playerStart.x + step * 2, playerStart.y));
    br20368_farTotem = placeHostileMonster(MK_GOBLIN_TOTEM,
                                            posAt(playerStart.x + step * 20, playerStart.y));
    TEST_ASSERT(br20368_nearTotem != NULL, "setup: the near totem should spawn");
    TEST_ASSERT(br20368_farTotem != NULL, "setup: the far totem should spawn");
    putMonsterToSleep(br20368_nearTotem);
    putMonsterToSleep(br20368_farTotem);
    TEST_ASSERT(monsterIsAsleep(br20368_nearTotem), "setup: the near totem should start inert");
    TEST_ASSERT(monsterIsAsleep(br20368_farTotem), "setup: the far totem should start inert");
}

static boolean br20368_step(void) {
    monsterTakesTurn(br20368_nearTotem);
    monsterTakesTurn(br20368_farTotem);
    return false;
}

static boolean br20368_totem_aware_only_within_range(void) {
    TEST_ASSERT(monsterIsTrackingPlayer(br20368_nearTotem),
                "a totem within its detection range should become aware of you and start hunting");
    TEST_ASSERT(!monsterIsAsleep(br20368_nearTotem),
                "a totem within its detection range should not stand inert");
    TEST_ASSERT(monsterIsAsleep(br20368_farTotem),
                "a totem outside its detection range should stand inert as though asleep");
    TEST_ASSERT(!monsterIsTrackingPlayer(br20368_farTotem),
                "a totem outside its detection range should not become aware of you");
    return !testFailed;
}

// ============================================================
// BR44597 — While you are hallucinating and not telepathic,
// monsters you can see are displayed under the name of a
// randomly chosen monster kind.
//
// The sidebar draws each listed monster's name through the same
// naming the game uses everywhere, and every redraw picks a fresh
// random kind, so the name is sampled several times.  A control
// sample before the hallucination starts proves the monster is
// listed under its true name to begin with.  A second control
// then turns telepathy on, which must restore the true name even
// though the hallucination is still running.
// ============================================================

#define BR44597_TRUE_NAME "Kraken"
#define BR44597_SAMPLES 6

static boolean br44597_controlChecked;
static boolean br44597_telepathyChecked;
static int br44597_trueNameCount;
static int br44597_otherNameCount;

static void br44597_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    placeHostileMonster(MK_KRAKEN, posOffset(getPlayerPos(), 1, 0));
    br44597_controlChecked = false;
    br44597_telepathyChecked = false;
    br44597_trueNameCount = 0;
    br44597_otherNameCount = 0;
}

static boolean br44597_step(void) {
    if (!br44597_controlChecked) {
        // Control: with clear senses the visible monster is listed
        // under its true kind name.
        TEST_ASSERT(sidebarContains(BR44597_TRUE_NAME),
                    "control: a monster you can see is listed under its true name");
        br44597_controlChecked = true;
        applyStatusToPlayer(STATUS_HALLUCINATING, 100);
        return true;
    }
    if (br44597_trueNameCount + br44597_otherNameCount < BR44597_SAMPLES) {
        // Each sidebar redraw while hallucinating picks a fresh name.
        if (sidebarContains(BR44597_TRUE_NAME)) {
            br44597_trueNameCount++;
        } else {
            br44597_otherNameCount++;
        }
        return true;
    }
    if (!br44597_telepathyChecked) {
        // The rule holds only while you are not telepathic: with
        // telepathy on, the true name must come back.
        applyStatusToPlayer(STATUS_TELEPATHIC, 10);
        TEST_ASSERT(sidebarContains(BR44597_TRUE_NAME),
                    "telepathy restores the true name while hallucinating");
        br44597_telepathyChecked = true;
    }
    return false;
}

static boolean br44597_verify(void) {
    TEST_ASSERT(br44597_controlChecked,
                "control ran: the true name was listed before hallucinating");
    TEST_ASSERT(playerHasStatus(STATUS_HALLUCINATING),
                "you are hallucinating throughout the sampling");
    TEST_ASSERT(!playerHasStatus(STATUS_TELEPATHIC) || br44597_telepathyChecked,
                "telepathy was off during the sampling");
    TEST_ASSERT_EQ(br44597_trueNameCount + br44597_otherNameCount,
                   BR44597_SAMPLES, "sidebar name samples observed");
    TEST_ASSERT(br44597_otherNameCount >= 1,
                "while hallucinating and not telepathic, a visible monster is "
                "displayed under a randomly chosen monster kind name");
    TEST_ASSERT(br44597_telepathyChecked,
                "the telepathy check ran");
    return !testFailed;
}

// ---------------------------------------------
// BR61713 — a hunting monster that can blink teleports up the scent
// trail toward you
// ---------------------------------------------

static creature *br61713_guardian;
static int br61713_startDistance;

static int br61713_chebyshev(pos a, pos b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return (dx > dy ? dx : dy);
}

static void br61713_setup(void) {
    clearArena();
    // An open corridor east of you: the blink's candidate squares and the
    // scent gradient both run along it.
    for (int i = 1; i <= 5; i++) {
        setTerrain(posOffset(getPlayerPos(), i, 0), FLOOR);
    }
    // A player turn end lays the scent trail (updateScent() runs as the turn
    // ends), before there is a monster to act on it.
    restOneTurn();
    // The winged guardian carries MONST_ALWAYS_USE_ABILITY with its blinking
    // bolt, so its hunting turn attempts the blink outright instead of
    // rolling the usual 30 percent chance.
    br61713_guardian = placeHostileMonster(MK_WINGED_GUARDIAN,
                                           posOffset(getPlayerPos(), 4, 0));
    br61713_startDistance = br61713_chebyshev(monsterPosition(br61713_guardian),
                                              getPlayerPos());
}

static boolean br61713_step(void) {
    // Give the guardian its hunting turn outright - its blink message is
    // buffered as a combat message - then end a player turn, whose schedule
    // flushes buffered combat messages into the log.
    monsterTakesTurn(br61713_guardian);
    restOneTurn();
    return false;
}

static boolean br61713_verify(void) {
    TEST_ASSERT(monsterIsTrackingPlayer(br61713_guardian),
                "precondition: the guardian is hunting you");
    TEST_ASSERT(messageWasShown("blinks"),
                "a hunting monster that can blink should blink instead of walking");
    int endDistance = br61713_chebyshev(monsterPosition(br61713_guardian),
                                         getPlayerPos());
    TEST_ASSERT(endDistance < br61713_startDistance,
                "the blink should carry it up the scent trail, closer to you");
    return !testFailed;
}

// ---------------------------------------------
// BR61728 — an allied monster that can blink teleports
// toward the nearest enemy, using a distance map it constructs
// ---------------------------------------------

static creature *br61728_ally;
static creature *br61728_nearest;
static creature *br61728_far;
static pos br61728_allyBefore, br61728_allyAfter;
static int br61728_nearestBefore, br61728_nearestAfter;
static int br61728_farBefore, br61728_farAfter;
static int br61728_maxJump;

static int br61728_chebyshev(pos a, pos b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return (dx > dy ? dx : dy);
}

static void br61728_setup(void) {
    setArenaRadius(16, FLOOR);
    pos p = getPlayerPos();
    // The ally three squares from you, its nearest enemy twelve squares
    // beyond it, and a visible enemy the other way fifteen squares from the
    // ally, so "nearest" is a real choice between two foes.  Pick the side of
    // you that has room for both enemies.
    boolean westRoom = (p.x - 15 >= 0 && p.x + 12 <= levelWidth() - 1);
    int dir = westRoom ? -1 : 1;
    TEST_ASSERT(westRoom || (p.x + 15 <= levelWidth() - 1 && p.x - 12 >= 0),
                "setup: there should be room for the enemy squares");

    restOneTurn(); // establish sight over the cleared floor

    br61728_ally = spawnMonster(MK_WINGED_GUARDIAN, posOffset(p, 3 * dir, 0));
    makeMonsterAlly(br61728_ally);
    makeMonsterFollowPlayer(br61728_ally);
    makeMonsterNotFollowPlayer(br61728_ally); // holds no leash: it acts on enemies wherever they are
    TEST_ASSERT(cellIsVisible(monsterPosition(br61728_ally)),
                "setup: the ally must stand within sight of the player");

    // The nearest enemy is invisible: the ally's scan for the enemy it walks
    // toward notices an invisible foe only by chance, while the distance map
    // it builds for its blink sees every enemy.  A blink therefore heads for
    // the invisible nearest enemy instead of the visible farther one.
    br61728_nearest = spawnMonster(MK_RAT, posOffset(p, 15 * dir, 0));
    makeMonsterHostile(br61728_nearest);
    applyStatusToMonster(br61728_nearest, STATUS_INVISIBLE, 50);

    br61728_far = placeHostileMonster(MK_RAT, posOffset(p, -12 * dir, 0));
}

static boolean br61728_step(void) {
    br61728_allyBefore = monsterPosition(br61728_ally);
    br61728_nearestBefore = br61728_chebyshev(br61728_allyBefore,
                                              monsterPosition(br61728_nearest));
    br61728_farBefore = br61728_chebyshev(br61728_allyBefore,
                                          monsterPosition(br61728_far));

    // Give the ally turns until it blinks.  On a turn where its enemy scan
    // happens to notice the invisible foe it just steps one square toward
    // it; each turn re-rolls that scan, and a blink follows with overwhelming
    // probability well within the cap.
    pos prev = br61728_allyBefore;
    br61728_maxJump = 0;
    for (int turn = 0; turn < 40; turn++) {
        monsterTakesTurn(br61728_ally);
        pos now = monsterPosition(br61728_ally);
        int jump = br61728_chebyshev(now, prev);
        if (jump > br61728_maxJump) {
            br61728_maxJump = jump;
        }
        prev = now;
        if (jump > 1) {
            break; // a blink, not a step
        }
    }
    br61728_allyAfter = prev;
    br61728_nearestAfter = br61728_chebyshev(br61728_allyAfter,
                                             monsterPosition(br61728_nearest));
    br61728_farAfter = br61728_chebyshev(br61728_allyAfter,
                                         monsterPosition(br61728_far));

    // End a player turn: its schedule flushes the buffered "blinks" combat
    // message into the log.  All distances are already captured above, so
    // whatever the creatures do during that turn cannot change them.
    restOneTurn();
    return false;
}

static boolean br61728_ally_blinks_toward_nearest_enemy(void) {
    TEST_ASSERT(monsterIsAlly(br61728_ally), "precondition: the subject is an ally");
    TEST_ASSERT_EQ(br61728_nearestBefore, 12,
                   "precondition: the nearest enemy is twelve squares beyond the ally");
    TEST_ASSERT_EQ(br61728_farBefore, 15,
                   "precondition: the other enemy is fifteen squares from the ally");
    TEST_ASSERT(messageWasShown("blinks"),
                "an allied monster that can blink should blink instead of walking");
    TEST_ASSERT(!posEq(br61728_allyAfter, br61728_allyBefore),
                "the blink should teleport the ally to a new square");
    TEST_ASSERT(br61728_maxJump > 1,
                "the blink should jump several squares at once, not step one");
    TEST_ASSERT(br61728_nearestAfter < br61728_nearestBefore,
                "the blink should carry the ally toward its nearest enemy");
    TEST_ASSERT(br61728_farAfter > br61728_farBefore,
                "the blink should not carry the ally toward the farther enemy");
    return !testFailed;
}

// ---------------------------------------------
// BR75632 — each of a monster's timed conditions, such as slowness,
// haste, levitation, invisibility, poisoning and discord, counts down
// by one per turn
// ---------------------------------------------

static creature *br75632_subject;
static creature *br75632_hasted;

static void br75632_setup(void) {
    setArenaRadius(8, FLOOR);
    pos p = getPlayerPos();
    // Well away from you, on a line where every step stays on cleared floor.
    int dx = (p.x + 7 <= levelWidth()) ? 6 : -6;

    // One monster carries five of the conditions the rule names.
    br75632_subject = spawnMonster(MK_OGRE, posOffset(p, dx, 0));
    slowMonster(br75632_subject, 10);
    poisonMonster(br75632_subject, 10, 1);
    applyStatusToMonster(br75632_subject, STATUS_LEVITATING, 10);
    applyStatusToMonster(br75632_subject, STATUS_INVISIBLE, 10);
    applyStatusToMonster(br75632_subject, STATUS_DISCORDANT, 10);

    // Slowness and haste cancel each other out, so haste goes on a second
    // monster.  It is kept from acting at all: while you rest, the ally's
    // leash stretches past the discordant ogre, which it may glimpse
    // through the ogre's invisibility, and a free-roaming rat would then
    // blunder into the ogre's reach.  The countdown runs for every monster
    // every turn whether it acts or not.
    br75632_hasted = spawnMonster(MK_RAT, posOffset(p, dx < 0 ? 2 : -2, 0));
    makeMonsterAlly(br75632_hasted);
    hasteMonster(br75632_hasted, 10);
    applyStatusToMonster(br75632_hasted, STATUS_PARALYZED, 20);

    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_SLOWED), 10,
                   "setup: the subject should start with 10 turns of slowness");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_POISONED), 10,
                   "setup: the subject should start with 10 turns of poisoning");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_LEVITATING), 10,
                   "setup: the subject should start with 10 turns of levitation");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_INVISIBLE), 10,
                   "setup: the subject should start with 10 turns of invisibility");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_DISCORDANT), 10,
                   "setup: the subject should start with 10 turns of discord");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_hasted, STATUS_HASTED), 10,
                   "setup: the second monster should start with 10 turns of haste");
}

static boolean br75632_countdown_step(void) {
    // Five player turns, each of which ends one round of turns for every
    // monster on the level.
    for (int i = 0; i < 5; i++) {
        restOneTurn();
    }
    return false;
}

static boolean monster_conditions_count_down_by_one_per_turn(void) {
    TEST_ASSERT(monsterIsAlive(br75632_subject),
                "setup: the subject should survive the counted turns");
    TEST_ASSERT(monsterIsAlive(br75632_hasted),
                "setup: the hasted monster should survive the counted turns");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_SLOWED), 5,
                   "slowness should count down by one per turn");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_POISONED), 5,
                   "poisoning should count down by one per turn");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_LEVITATING), 5,
                   "levitation should count down by one per turn");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_INVISIBLE), 5,
                   "invisibility should count down by one per turn");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_subject, STATUS_DISCORDANT), 5,
                   "discord should count down by one per turn");
    TEST_ASSERT_EQ(monsterStatusTurns(br75632_hasted, STATUS_HASTED), 5,
                   "haste should count down by one per turn");
    return !testFailed;
}

// ---------------------------------------------
// BR29907 — an ally told not to follow you stays where it is and
// mills about, though it still chases enemies that come within its
// range
// ---------------------------------------------

static creature *br29907_ally;
static creature *br29907_enemy;
static int br29907_movedTurns;

static void br29907_setup(void) {
    setArenaRadius(8, FLOOR);
    pos p = getPlayerPos();
    // Beyond the 4-square leash and the 3-square idle radius, so only the
    // order not to follow can explain where it stays.
    int dx = (p.x + 7 <= levelWidth()) ? 6 : -6;
    pos post = posOffset(p, dx, 0);
    TEST_ASSERT(cellIsTerrain(post, FLOOR),
                "setup: the ally's post should be cleared floor");

    br29907_ally = spawnMonster(MK_RAT, post);
    makeMonsterAlly(br29907_ally);
    makeMonsterFollowPlayer(br29907_ally);
    makeMonsterNotFollowPlayer(br29907_ally); // told not to follow you
    TEST_ASSERT(monsterIsAlly(br29907_ally), "setup: the subject should be an ally");
}

static boolean br29907_hold_and_chase_step(void) {
    // Phase 1: with no enemy around it mills about, stepping on only about
    // one turn in three.  Each turn starts back at its post, so the dice
    // that decide its step cannot wander it off the cleared floor, and so
    // that an ally which simply tracked you instead would step on every
    // single one of the 200 turns.
    pos post = monsterPosition(br29907_ally);
    br29907_movedTurns = 0;
    for (int i = 0; i < 200; i++) {
        monsterTakesTurn(br29907_ally);
        if (!posEq(monsterPosition(br29907_ally), post)) {
            br29907_movedTurns++;
        }
        setMonsterPos(br29907_ally, post);
    }

    // Phase 2: an enemy comes within its range.  It is placed to the side
    // of the post, off the line back to you, so only the chase can bring
    // the ally to it.
    pos enemyLoc = posOffset(post, 0, (post.y + 5 < levelHeight()) ? 5 : -5);
    TEST_ASSERT(cellIsTerrain(enemyLoc, FLOOR),
                "setup: the enemy should stand on cleared floor");
    br29907_enemy = placeHostileMonster(MK_RAT, enemyLoc);
    if (br29907_enemy == NULL) {
        testFail("setup: the enemy could not be placed");
        return false;
    }
    for (int i = 0; i < 15; i++) {
        monsterTakesTurn(br29907_ally);
    }
    return false;
}

static boolean ally_holds_post_and_still_chases(void) {
    TEST_ASSERT(br29907_movedTurns >= 35 && br29907_movedTurns <= 100,
                "an ally told not to follow you should mill about, stepping on "
                "only a minority of its turns instead of tracking you every turn");
    if (br29907_enemy == NULL) {
        return false;
    }
    TEST_ASSERT(monsterIsHurt(br29907_enemy) || monsterIsDead(br29907_enemy),
                "an ally told not to follow you should still chase an enemy "
                "that comes within its range");
    return !testFailed;
}

// ============================================================
// BR58883 — an attack bolt is not fired at a creature embedded
// in a wall.
//
// An allied arrow turret faces a hostile rat along a clear
// east–west line north of you.  An ally never targets you
// with an attack bolt, so the rat is the turret's only candidate;
// the turret is immobile, so every one of its turns tries the
// bolt.  With the rat on open floor its arrows land (control);
// wall the rat in — its cell now obstructs passability, and a
// rat is no creature that can be attacked through walls — and
// over the same number of turret turns not one bolt is fired;
// restoring the floor brings the arrows back (control).  The
// observable is the rat's health, so each open-floor phase gives
// the cast a hundred turns to land while the walled phase must
// produce no damage at all.
// ============================================================

static creature *br58883_turret;
static creature *br58883_target;
static pos br58883_targetPos;
static int br58883_healthStart;
static boolean br58883_openShot, br58883_wallShot, br58883_againShot;

static void br58883_setup(void) {
    setArenaRadius(6, FLOOR);
    br58883_targetPos = posOffset(getPlayerPos(), 3, 1);

    // The caster: an allied arrow turret.  (MONST_TURRET includes
    // MONST_IMMOBILE, so the turret tries its bolt every turn instead of
    // moving, and as an ally it never bolts you.)
    br58883_turret = spawnMonster(MK_ARROW_TURRET, posOffset(getPlayerPos(), -4, 1));
    TEST_ASSERT(br58883_turret != NULL, "setup: the arrow turret should spawn");
    if (br58883_turret == NULL) {
        return;
    }
    makeMonsterAlly(br58883_turret);
    revealCell(monsterPosition(br58883_turret));

    // The target: a hostile rat on the same unobstructed line.
    br58883_target = placeHostileMonster(MK_RAT, br58883_targetPos);
    TEST_ASSERT(br58883_target != NULL, "setup: the rat should spawn");
    if (br58883_target == NULL) {
        return;
    }
    setMonsterHealth(br58883_target, 100);
    br58883_healthStart = monsterHealth(br58883_target);
}

static boolean br58883_step(void) {
    // Control: the rat stands in the open — the turret should shoot it.
    for (int i = 0;
         i < 100 && monsterHealth(br58883_target) == br58883_healthStart;
         i++) {
        monsterTakesTurn(br58883_turret);
    }
    br58883_openShot = monsterHealth(br58883_target) < br58883_healthStart;

    // The rule's scenario: embed the rat in a wall.  (setTerrain clears the
    // cell's creature flag, so the rat is re-placed on it afterwards.)
    setTerrain(br58883_targetPos, GRANITE);
    setMonsterPos(br58883_target, br58883_targetPos);
    int beforeWall = monsterHealth(br58883_target);
    for (int i = 0; i < 100; i++) {
        monsterTakesTurn(br58883_turret);
    }
    br58883_wallShot = monsterHealth(br58883_target) < beforeWall;

    // Control again: restore the floor under the rat.
    setTerrain(br58883_targetPos, FLOOR);
    setMonsterPos(br58883_target, br58883_targetPos);
    for (int i = 0;
         i < 100 && monsterHealth(br58883_target) == beforeWall;
         i++) {
        monsterTakesTurn(br58883_turret);
    }
    br58883_againShot = monsterHealth(br58883_target) < beforeWall;
    return false;
}

static boolean br58883_no_attack_bolt_at_embedded_creature(void) {
    TEST_ASSERT(br58883_openShot,
                "control: the turret should shoot a creature standing in the open");
    TEST_ASSERT(!br58883_wallShot,
                "an attack bolt should not be fired at a creature embedded in a wall");
    TEST_ASSERT(br58883_againShot,
                "control: the turret should shoot again once the creature is out of the wall");
    return !testFailed;
}

// ============================================================
// BR57010 — a monster will not shoot a fireball from ground that
// could combust into something harmful
//
// A flame turret is immobile and has only a fire bolt, so its
// only possible action is to cast at the hostile rat down the
// corridor.  Its own tile is the variable: plain floor (the
// bolt's fire is no threat to the turret) versus grass, which
// burns into fire the turret itself avoids.
// ============================================================

static creature *br57010_turret;
static creature *br57010_target;
static pos br57010_turretPos;
static int br57010_healthStart;
static boolean br57010_floorCast, br57010_grassCast, br57010_floorCastAgain;

static void br57010_setup(void) {
    setArenaRadius(6, FLOOR);
    br57010_turretPos = posOffset(getPlayerPos(), -4, 1);

    br57010_target = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 3, 1));
    TEST_ASSERT(br57010_target != NULL, "setup: the rat should spawn");
    if (br57010_target == NULL) {
        return;
    }
    setMonsterHealth(br57010_target, 100);
    br57010_healthStart = monsterHealth(br57010_target);

    // An allied flame turret: MONST_TURRET includes MONST_IMMOBILE, so it
    // tries its bolt every turn instead of moving, and as an ally it never
    // bolts you.
    br57010_turret = spawnMonster(MK_FLAME_TURRET, br57010_turretPos);
    TEST_ASSERT(br57010_turret != NULL, "setup: the flame turret should spawn");
    if (br57010_turret == NULL) {
        return;
    }
    makeMonsterAlly(br57010_turret);
    revealCell(br57010_turretPos);
}

static boolean br57010_step(void) {
    // Control: the turret stands on plain floor, so its fire bolt can land.
    for (int i = 0;
         i < 100 && monsterHealth(br57010_target) == br57010_healthStart;
         i++) {
        monsterTakesTurn(br57010_turret);
    }
    br57010_floorCast = monsterHealth(br57010_target) < br57010_healthStart;

    // The rule's scenario: put the turret on grass.  setTerrain clears the
    // cell's creature flag, so the turret is re-placed on it afterwards.
    setTerrain(br57010_turretPos, GRASS);
    setMonsterPos(br57010_turret, br57010_turretPos);
    int beforeGrass = monsterHealth(br57010_target);
    for (int i = 0; i < 100; i++) {
        monsterTakesTurn(br57010_turret);
    }
    br57010_grassCast = monsterHealth(br57010_target) < beforeGrass;

    // Control again: restore the floor under the turret.
    setTerrain(br57010_turretPos, FLOOR);
    setMonsterPos(br57010_turret, br57010_turretPos);
    for (int i = 0;
         i < 100 && monsterHealth(br57010_target) == beforeGrass;
         i++) {
        monsterTakesTurn(br57010_turret);
    }
    br57010_floorCastAgain = monsterHealth(br57010_target) < beforeGrass;
    return false;
}

static boolean br57010_no_fireball_from_flammable_ground(void) {
    TEST_ASSERT(br57010_floorCast,
                "control: the turret should fire its bolt from plain floor");
    TEST_ASSERT(!br57010_grassCast,
                "a monster should not shoot a fireball from ground that could combust into something harmful");
    TEST_ASSERT(br57010_floorCastAgain,
                "control: the turret should fire again once the ground under it is safe");
    return !testFailed;
}

// ---------------------------------------------
// BR63549 — a water-bound grappler does not surface on the turn it grabs
// ---------------------------------------------

static creature *br63549_grappler;
static creature *br63549_prey;
static boolean br63549_submergedBefore;
static boolean br63549_submergedAfterGrab;
static boolean br63549_submergedAfterSecondAttack;

static void br63549_setup(void) {
    setArenaRadius(6, FLOOR);
    pos p = getPlayerPos();
    pos grapplerPos = posOffset(p, 3, 0);
    pos preyPos = posOffset(p, 4, 0);

    // The grappler must stay in liquid, so both it and its prey stand in deep
    // water.  Discord makes the grappler pick the adjacent prey over you; the
    // prey itself need not be able to hurt anyone.
    setTerrain(grapplerPos, DEEP_WATER);
    setTerrain(preyPos, DEEP_WATER);

    br63549_grappler = spawnMonster(MK_BOG_MONSTER, grapplerPos);
    makeMonsterHostile(br63549_grappler);
    applyStatusToMonster(br63549_grappler, STATUS_DISCORDANT, 100);
    setMonsterSubmerged(br63549_grappler, true);

    br63549_prey = spawnMonster(MK_RAT, preyPos);
    makeMonsterHostile(br63549_prey);

    // Submersion is what hides the grappler from a dry observer.
    br63549_submergedBefore = monsterIsHiddenFrom(br63549_grappler, &player);
}

static boolean br63549_step(void) {
    monsterTakesTurn(br63549_grappler);
    br63549_submergedAfterGrab = monsterIsHiddenFrom(br63549_grappler, &player);

    // A second attack, now that the grappler is already holding its prey,
    // shows the contrast: only the grabbing turn keeps it under the surface.
    monsterTakesTurn(br63549_grappler);
    br63549_submergedAfterSecondAttack = monsterIsHiddenFrom(br63549_grappler, &player);
    return false;
}

static boolean br63549_stays_submerged_on_grab_turn(void) {
    TEST_ASSERT(br63549_submergedBefore,
                "setup: the grappler should start hidden beneath the water");
    TEST_ASSERT(br63549_submergedAfterGrab,
                "a water-bound grappler should not surface on the turn it grabs its prey");
    TEST_ASSERT(!br63549_submergedAfterSecondAttack,
                "control: a grappler that is already holding its prey should surface when it attacks again");
    return !testFailed;
}

// ---------------------------------------------
// BR64668 — an always-hunting monster that has given up on your scent paths
// straight toward you
//
// A stone guardian (MONST_ALWAYS_HUNTING) starts behind a wall, so the
// player's scent never reaches it and it is out of sight: at that spot scent
// direction comes up blank and the local scent is already maximal.  An
// always-hunting monster in that position gives up on the scent trail and
// paths toward the player's actual position instead, so it walks around the
// wall and reaches you.  Without that behavior the monster would drop into
// a wander and would not close on a player it has never seen.  The control
// assertions pin the trait and the scent-free, unseen start.
// ---------------------------------------------

#define BR64668_MAX_TURNS 30

static creature *br64668_guardian;
static pos br64668_start;
static int br64668_turns;

static void br64668_setup(void) {
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(8, FLOOR);

    // A wall directly between you and the guardian shadows it from your
    // scent and your sight, but the open arena leaves a route around both
    // ends of the wall.
    for (int dy = -2; dy <= 2; dy++) {
        setTerrain(posOffset(getPlayerPos(), 3, dy), WALL);
    }
    // Lay your scent over the new arena; the wall keeps it away from the
    // guardian's tile.
    restOneTurn();

    br64668_guardian = spawnMonster(MK_GUARDIAN,
                                    posOffset(getPlayerPos(), 5, 0));
    makeMonsterHostile(br64668_guardian);
    br64668_start = monsterPosition(br64668_guardian);
    br64668_turns = 0;
}

static boolean br64668_step(void) {
    monsterTakesTurn(br64668_guardian);
    br64668_turns++;
    return !monsterIsAdjacentToPlayer(br64668_guardian)
           && br64668_turns < BR64668_MAX_TURNS;
}

static boolean br64668_given_up_monster_paths_to_player(void) {
    TEST_ASSERT(br64668_guardian != NULL, "the guardian should have spawned");
    TEST_ASSERT(monsterHasTrait(br64668_guardian, MONST_ALWAYS_HUNTING),
                "control: the guardian's catalog entry is always hunting");
    TEST_ASSERT(!cellIsVisible(br64668_start),
                "control: the guardian starts out of your sight behind the wall");
    TEST_ASSERT(monsterIsAdjacentToPlayer(br64668_guardian),
                "an always-hunting monster that has given up on your scent "
                "should path toward you and reach you around the wall");
    return !testFailed;
}

// ============================================================
// BR70006 — a monster keeps at least six squares away from a
// weapon-immune or invulnerable enemy that can move, but not from an
// immobile one.
// ============================================================

static int br70006_chebyshev(pos a, pos b) {
    int dx = a.x - b.x; if (dx < 0) dx = -dx;
    int dy = a.y - b.y; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

static creature *br70006_subject;
static creature *br70006_enemy;
static int br70006_before;

// Lay a clear arena and put the subject and its enemy on the same row,
// `subjectDx` and `enemyDx` squares from you.
static void br70006_scene(enum monsterTypes subjectType, boolean subjectIsAlly,
                          enum monsterTypes enemyType, boolean enemyIsAlly,
                          int subjectDx, int enemyDx) {
    clearMonsters();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(9, FLOOR);
    // Aggravate yourself and run one real turn: this pins the engine's
    // stealth range high so the monsters stay aware of you, and recomputes
    // the field of view over the cleared arena.
    applyStatusToPlayer(STATUS_AGGRAVATING, 10000);
    restOneTurn();

    pos p = getPlayerPos();
    for (int dx = -9; dx <= 9; dx++) {
        for (int dy = -9; dy <= 9; dy++) {
            revealCell(posOffset(p, dx, dy));
        }
    }
    br70006_subject = spawnMonster(subjectType, posOffset(p, subjectDx, 0));
    if (subjectIsAlly) {
        makeMonsterAlly(br70006_subject);
    } else {
        makeMonsterHostile(br70006_subject);
    }
    br70006_enemy = spawnMonster(enemyType, posOffset(p, enemyDx, 0));
    if (enemyIsAlly) {
        makeMonsterAlly(br70006_enemy);
    } else {
        makeMonsterHostile(br70006_enemy);
    }
    br70006_before = br70006_chebyshev(monsterPosition(br70006_subject),
                                       monsterPosition(br70006_enemy));
}

static boolean br70006_take_turn(void) {
    monsterTakesTurn(br70006_subject);
    return false;
}

// An ally five squares from a hostile weapon-immune enemy that can move
// must give ground rather than close in.
static void br70006_mobile_ally_setup(void) {
    br70006_scene(MK_RAT, true, MK_REVENANT, false, 2, 7);
}

static boolean br70006_mobile_ally_verify(void) {
    TEST_ASSERT(br70006_subject != NULL && br70006_enemy != NULL,
                "the ally and its enemy should have spawned");
    TEST_ASSERT(monsterHasTrait(br70006_enemy, MONST_IMMUNE_TO_WEAPONS),
                "control: the enemy is immune to weapons");
    TEST_ASSERT(!monsterHasTrait(br70006_enemy, MONST_IMMOBILE),
                "control: the enemy can move");
    TEST_ASSERT_EQ(br70006_before, 5,
                   "setup: the enemy starts five squares away");
    TEST_ASSERT(br70006_chebyshev(monsterPosition(br70006_subject),
                                  monsterPosition(br70006_enemy)) >= 6,
                "an ally should back away to at least six squares from a "
                "mobile weapon-immune enemy");
    return !testFailed;
}

// The same retreat from an invulnerable enemy that can move.
static void br70006_invulnerable_ally_setup(void) {
    br70006_scene(MK_RAT, true, MK_WARDEN_OF_YENDOR, false, 2, 7);
}

static boolean br70006_invulnerable_ally_verify(void) {
    TEST_ASSERT(br70006_subject != NULL && br70006_enemy != NULL,
                "the ally and its enemy should have spawned");
    TEST_ASSERT(monsterHasTrait(br70006_enemy, MONST_INVULNERABLE),
                "control: the enemy is invulnerable");
    TEST_ASSERT(!monsterHasTrait(br70006_enemy, MONST_IMMOBILE),
                "control: the enemy can move");
    TEST_ASSERT_EQ(br70006_before, 5,
                   "setup: the enemy starts five squares away");
    TEST_ASSERT(br70006_chebyshev(monsterPosition(br70006_subject),
                                  monsterPosition(br70006_enemy)) >= 6,
                "an ally should back away to at least six squares from a "
                "mobile invulnerable enemy");
    return !testFailed;
}

// A hostile monster two squares from an allied weapon-immune enemy that
// can move turns and flees.
static void br70006_mobile_hostile_setup(void) {
    br70006_scene(MK_RAT, false, MK_REVENANT, true, 4, 2);
}

static boolean br70006_mobile_hostile_verify(void) {
    TEST_ASSERT(br70006_subject != NULL && br70006_enemy != NULL,
                "the monster and its enemy should have spawned");
    TEST_ASSERT_EQ(br70006_before, 2,
                   "setup: the enemy starts two squares away");
    TEST_ASSERT(monsterIsFleeing(br70006_subject),
                "a monster should turn and flee from a mobile weapon-immune "
                "enemy within six squares");
    return !testFailed;
}

// The same scene with an immobile weapon-immune enemy: it is not feared,
// so the monster keeps hunting instead of fleeing.
static void br70006_immobile_hostile_setup(void) {
    br70006_scene(MK_RAT, false, MK_MIRRORED_TOTEM, true, 4, 2);
}

static boolean br70006_immobile_hostile_verify(void) {
    TEST_ASSERT(br70006_subject != NULL && br70006_enemy != NULL,
                "the monster and its enemy should have spawned");
    TEST_ASSERT(monsterHasTrait(br70006_enemy, MONST_IMMUNE_TO_WEAPONS),
                "control: the enemy is immune to weapons");
    TEST_ASSERT(monsterHasTrait(br70006_enemy, MONST_IMMOBILE),
                "control: the enemy cannot move");
    TEST_ASSERT_EQ(br70006_before, 2,
                   "setup: the enemy starts two squares away");
    TEST_ASSERT(!monsterIsFleeing(br70006_subject),
                "a monster should not flee from an immobile weapon-immune enemy");
    TEST_ASSERT(monsterIsTrackingPlayer(br70006_subject),
                "a monster near an immobile weapon-immune enemy should keep hunting");
    return !testFailed;
}

// ============================================================
// BR64018 — hordes flagged as allied with you spawn their members
// as your allies.
//
// The stock horde table's allied entries carry a leader but no
// members, so the framework assembles an allied horde with two
// followers from the engine's own horde-spawning path.  Both the
// leader and every follower must come out on your side; the
// member-count check is the control that the follower branch was
// actually exercised rather than passing on the leader alone.
// ============================================================

static creature *br64018_leader;
static int br64018_members;

static void br64018_setup(void) {
    clearMonsters();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(8, FLOOR);
    br64018_leader = spawnAlliedHordeWithFollowers(posOffset(getPlayerPos(), 4, 0),
                                                   2, &br64018_members);
}

static boolean br64018_allied_horde_spawns_allies(void) {
    TEST_ASSERT(br64018_leader != NULL,
                "the allied horde should spawn a leader");
    TEST_ASSERT_EQ(br64018_members, 2,
                   "control: both members of the allied horde should spawn");
    TEST_ASSERT_EQ(monsterCount(), 3,
                   "the horde should consist of its leader and two members");
    TEST_ASSERT(monsterIsAlly(br64018_leader),
                "the leader of an allied horde should be your ally");
    for (int i = 0; i < monsterCount(); i++) {
        creature *member = monsterAtIndex(i);
        TEST_ASSERT(member != NULL && monsterIsAlly(member),
                    "every member of an allied horde should be your ally");
    }
    return !testFailed;
}

const testCase tests_monsters[] = {
    { "BR83001_webbed_monster_struggles_free_after_a_few_turns", webbed_monster_setup, webbed_monster_struggles_step, webbed_monster_escapes_after_a_few_turns },
    { "BR83001_web_immune_monster_walks_free_immediately",       web_immune_monster_setup, web_immune_monster_turn_step, web_immune_monster_walks_free_immediately },

    { "BR44825_monster_avoids_invulnerable_enemy", futile_invulnerable_setup, NULL, monster_avoids_invulnerable_enemy },
    { "BR44825_monster_attacks_normal_enemy",      futile_normal_setup,       NULL, monster_does_not_avoid_normal_enemy },

    { "BR83744_will_not_cast_at_hidden_target",      hidden_target_setup,    hidden_target_step, monster_will_not_cast_at_hidden_target },
    { "BR83744_will_not_cast_without_line_of_fire",  line_of_fire_setup,     line_of_fire_step, monster_will_not_cast_without_line_of_fire },
    { "BR83744_will_not_cast_at_submerged_target",   submerged_target_setup, submerged_target_step, monster_will_not_cast_at_submerged_target },
    { "BR92702_dormant_monster_hidden",               dormant_monster_setup,           NULL, dormant_monster_is_hidden },
    { "BR92702_invisible_monster_hidden",             invisible_monster_setup,         NULL, invisible_monster_is_hidden },
    { "BR92702_submerged_monster_hidden",             submerged_monster_setup,         NULL, submerged_monster_is_hidden },

    { "BR43781_invisible_ally_not_hidden",            invisible_ally_setup,            NULL, ally_is_not_hidden },
    { "BR43781_follower_not_hidden_from_leader",      invisible_follower_setup,        NULL, follower_is_not_hidden },

    { "BR97021_submerged_hidden_from_dry_observer",   submerged_with_dry_observer_setup, NULL, submerged_hidden_from_dry_observer },
    { "BR97021_submerged_seen_from_deep_water",       submerged_with_water_observer_setup, NULL, submerged_seen_from_deep_water },
    { "BR97021_levitating_observer_cannot_see_submerged", submerged_with_levitating_observer_setup, NULL, levitating_observer_cannot_see_submerged },

    { "BR63373_telepathy_reveals_living_monster",     telepathy_living_setup,          NULL, telepathy_reveals_living_monster },
    { "BR63373_telepathy_skips_inanimate_monster",    telepathy_inanimate_setup,       NULL, telepathy_skips_inanimate_monster },

    { "BR44440_entranced_monster_always_revealed",    entranced_revealed_setup,        NULL, entranced_monster_is_revealed },

    { "BR43957_visible_monster_known_hidden_monster_unknown", known_monster_setup,              NULL, visible_monster_known_hidden_monster_unknown },
    { "BR43957_distant_monster_unknown",                       distant_monster_setup,           NULL, distant_monster_unknown },
    { "BR43957_revealed_distant_monster_known",                revealed_distant_monster_setup,  NULL, revealed_distant_monster_known },
    { "BR43957_hidden_monster_stays_unknown_when_revealed",    hidden_revealed_monster_setup,   NULL, hidden_monster_stays_unknown_when_revealed },

    { "BR78631_entranced_monster_attacks_non_ally",   entranced_attacker_setup,        NULL, entranced_attacks_non_ally },
    { "BR78631_entranced_monster_attacks_player",     entranced_attacker_setup,        NULL, entranced_attacks_player },

    { "BR44999_player_spares_ally",                   ally_setup,                      NULL, player_spares_ally },
    { "BR44999_player_attacks_discordant_ally",       discordant_ally_setup,           NULL, player_attacks_discordant_ally },

    { "BR45066_ally_spares_entranced_creature",       ally_and_entranced_setup,        NULL, ally_spares_entranced_creature },

    { "BR45346_nobody_attacks_a_captive",             nobody_attacks_setup,            NULL, nobody_attacks_a_captive },

    { "BR54066_discordant_attacks_own_kind",          discordant_setup,                NULL, discordant_attacks_own_kind },
    { "BR54066_discordant_attacks_player",            discordant_setup,                NULL, discordant_attacks_player },

    { "BR45704_captive_is_never_enemy",               captive_setup,                   NULL, captive_is_never_enemy },
    { "BR45704_discord_makes_mutual_enemies",         discordant_ally_pair_setup,      NULL, discord_makes_mutual_enemies },
    { "BR45704_opposite_sides_are_enemies",           opposite_sides_setup,            NULL, opposite_sides_are_enemies },

    { "BR45522_you_and_ally_are_teammates",           ally_setup,                      NULL, you_and_ally_are_teammates },
    { "BR45522_allies_are_teammates",                 ally_pair_setup,                 NULL, allies_are_teammates },
    { "BR45522_followers_of_one_leader_are_teammates", followers_of_one_leader_setup,  NULL, followers_are_teammates },
    { "BR45522_monsters_without_a_shared_leader_are_not_teammates", no_shared_leader_setup, NULL, monsters_without_a_shared_leader_are_not_teammates },

    { "BR89577_eel_attacks_creature_in_deep_water",   eel_and_water_creature_setup,    NULL, eel_attacks_creature_in_deep_water },
    { "BR89577_eel_ignores_levitating_creature",      eel_and_levitating_creature_setup, NULL, eel_ignores_levitating_creature },

    { "BR11561_monster_avoids_stairs",                stairs_setup,                    NULL, monster_avoids_stairs },
    { "BR11561_player_ignores_stairs",                stairs_setup,                    NULL, player_ignores_stairs },

    { "BR15087_liquid_monster_avoids_dry_land",       liquid_and_dry_land_setup,       NULL, liquid_monster_avoids_dry_land },
    { "BR15087_liquid_monster_accepts_water",         liquid_and_water_setup,          NULL, liquid_monster_accepts_water },

    { "BR50228_hostile_monster_does_not_avoid_player", avoid_player_setup,             NULL, hostile_monster_does_not_avoid_player },

    { "BR27232_monster_avoids_lava",                  lava_setup,                      NULL, monster_avoids_lava },
    { "BR27232_fire_immune_monster_ignores_lava",     fire_immune_lava_setup,          NULL, fire_immune_monster_ignores_lava },

    { "BR52904_burning_wanderer_avoids_flammable_ground", burning_wanderer_setup,     NULL, burning_wanderer_avoids_flammable_ground },
    { "BR52904_burning_tracker_ignores_flammable_ground", burning_tracker_setup,      NULL, burning_tracker_ignores_flammable_ground },
    { "BR52904_nonburning_wanderer_ignores_flammable_ground", nonburning_wanderer_setup, NULL, nonburning_wanderer_ignores_flammable_ground },

    { "BR51199_beached_liquid_monster_cannot_move",   beached_liquid_monster_setup,    NULL, beached_liquid_monster_cannot_move },

    { "BR44948_never_sleeping_monster_starts_awake",  never_sleeping_monster_setup,    NULL, never_sleeping_monster_starts_awake },

    { "BR67313_raised_ally_placed_on_safe_ground",    raised_ally_safe_ground_setup,   NULL, raised_ally_placed_on_safe_ground },
    { "BR67313_raised_ally_avoids_harmful_and_occupied", raised_ally_avoids_harmful_setup, NULL, raised_ally_avoids_harmful_terrain_and_occupants },

    { "BR39283_summons_appear_at_distance", summoned_at_distance_setup, summoned_at_distance_step, summoned_members_appear_at_distance },

    { "BR51745_steps_onto_adjacent_fightable_enemy", fightable_adjacent_setup, NULL, monster_steps_onto_fightable_adjacent },
    { "BR51745_avoids_adjacent_futile_enemy",        futile_adjacent_setup,     NULL, monster_avoids_futile_adjacent },

    { "BR52591_diagonal_step_first",                 br52591_diagonal_setup,  br52591_take_turn_step, br52591_diagonal_step_taken },
    { "BR52591_cardinal_step_fallback",              br52591_cardinal_setup,  br52591_take_turn_step, br52591_cardinal_step_taken },
    { "BR52591_off_axis_step_fallback",              br52591_off_axis_setup,  br52591_take_turn_step, br52591_off_axis_step_taken },

    { "BR53265_wanderer_travels_between_checkpoints", br53265_setup, br53265_step, wanders_between_distant_checkpoints },

    { "BR66644_ally_idles_when_close_and_seen",      br66644_ally_setup, br66644_ally_step, ally_idles_when_close_and_seen },
    { "BR29907_ally_told_not_to_follow_holds_post",  br29907_setup, br29907_hold_and_chase_step, ally_holds_post_and_still_chases },
    { "BR75632_monster_conditions_count_down_per_turn", br75632_setup, br75632_countdown_step, monster_conditions_count_down_by_one_per_turn },

    { "BR54211_thief_without_item_returns_to_hunting", br54211_thief_setup, br54211_hunting_step, br54211_returns_to_hunting },
    { "BR54211_following_thief_without_item_becomes_ally", br54211_follower_setup, br54211_ally_step, br54211_returns_to_ally },

    { "BR46360_population_matches_the_depth_formula", NULL, NULL, population_matches_the_depth_formula },
    { "BR59340_ally_never_negates_player_in_reflection_armor", br59340_player_setup, br59340_player_step, br59340_player_verify },
    { "BR59340_ally_never_negates_an_allied_creature", br59340b_setup, br59340b_step, br59340b_verify },

    { "BR20368_totem_aware_only_within_detection_range", br20368_setup, br20368_step, br20368_totem_aware_only_within_range },

    { "BR44597_hallucinated_monster_names", br44597_setup, br44597_step, br44597_verify },

    { "BR64668_given_up_monster_paths_to_player", br64668_setup, br64668_step, br64668_given_up_monster_paths_to_player },

    { "BR61713_hunting_blinker_teleports_up_scent_trail", br61713_setup, br61713_step, br61713_verify },
    { "BR61728_ally_blinks_toward_nearest_enemy",   br61728_setup, br61728_step, br61728_ally_blinks_toward_nearest_enemy },

    { "BR58883_no_attack_bolt_at_embedded_creature", br58883_setup, br58883_step, br58883_no_attack_bolt_at_embedded_creature },

    { "BR57010_no_fireball_from_flammable_ground", br57010_setup, br57010_step, br57010_no_fireball_from_flammable_ground },

    { "BR63549_water_grappler_stays_submerged_on_grab", br63549_setup, br63549_step, br63549_stays_submerged_on_grab_turn },

    { "BR70006_ally_retreats_from_mobile_weapon_immune_enemy", br70006_mobile_ally_setup, br70006_take_turn, br70006_mobile_ally_verify },
    { "BR70006_ally_retreats_from_mobile_invulnerable_enemy",  br70006_invulnerable_ally_setup, br70006_take_turn, br70006_invulnerable_ally_verify },
    { "BR70006_hostile_flees_from_mobile_weapon_immune_enemy", br70006_mobile_hostile_setup, br70006_take_turn, br70006_mobile_hostile_verify },
    { "BR70006_ignores_immobile_weapon_immune_enemy",          br70006_immobile_hostile_setup, br70006_take_turn, br70006_immobile_hostile_verify },

    { "BR64018_allied_horde_spawns_allies", br64018_setup, NULL, br64018_allied_horde_spawns_allies },

    { NULL, NULL, NULL, NULL }
};
