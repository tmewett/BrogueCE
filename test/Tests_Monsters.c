// Tests derived from rules/04-monsters.md.
//
// These cover the deterministic, headless-observable monster rules: what hides
// a creature from an observer, who is willing to attack whom, allegiance
// (teammates and enemies), the terrain a monster refuses to enter, a beached
// liquid creature, and whether a never-sleeping monster starts awake.
//
// Omitted are everything that depends on level/horde generation, random
// chances (mutations, item drops, detection rolls, splitting, summoning
// counts), pathing choices, spell/bolt selection, messages and death text, and
// any rule that needs a screen or is otherwise not reproducible headlessly.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// ---------------------------------------------
// Shared helpers
// ---------------------------------------------

// Put a monster of the given kind on a chosen patch of terrain at a spot
// relative to the player.
static creature *monsterOn(enum monsterTypes type, int dx, int dy, short terrain) {
    pos p = posOffset(player.loc, dx, dy);
    setTerrain(p, terrain);
    return spawnMonster(type, p);
}

// Re-lay the player's own tile so its terrain is known.  setTerrain clears the
// whole cell, including the HAS_PLAYER flag, so the player is replaced on it.
static void standPlayerOn(short terrain) {
    pos p = player.loc;
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
// BR45346 — nobody attacks a captive
// ---------------------------------------------

static void captive_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    otherMonster = monsterOn(MK_RAT, 6, 0, FLOOR);
    makeMonsterCaptive(otherMonster);
}

static boolean nobody_attacks_a_captive(void) {
    TEST_ASSERT(!monsterCouldAttack(subject, otherMonster),
                "a monster should not attack a captive");
    TEST_ASSERT(!monsterCouldAttack(&player, otherMonster),
                "you should not attack a captive");
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
// BR51199 — a liquid creature on dry land writhes helplessly
// ---------------------------------------------

static void beached_eel_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = monsterOn(MK_EEL, 1, 0, FLOOR); // adjacent to the player
    makeMonsterHostile(subject);
    markedPos = monsterPosition(subject);
    markedHealth = playerHealth();
    monsterTakesTurn(subject);
}

static boolean beached_liquid_monster_cannot_move(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(subject), markedPos,
                       "a beached liquid creature should not move");
    TEST_ASSERT_EQ(playerHealth(), markedHealth,
                   "a beached liquid creature should not attack");
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

// ============================================================

const testCase tests_monsters[] = {
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

    { "BR78631_entranced_monster_attacks_non_ally",   entranced_attacker_setup,        NULL, entranced_attacks_non_ally },
    { "BR78631_entranced_monster_attacks_player",     entranced_attacker_setup,        NULL, entranced_attacks_player },

    { "BR44999_player_spares_ally",                   ally_setup,                      NULL, player_spares_ally },
    { "BR44999_player_attacks_discordant_ally",       discordant_ally_setup,           NULL, player_attacks_discordant_ally },

    { "BR45066_ally_spares_entranced_creature",       ally_and_entranced_setup,        NULL, ally_spares_entranced_creature },

    { "BR45346_nobody_attacks_a_captive",             captive_setup,                   NULL, nobody_attacks_a_captive },

    { "BR54066_discordant_attacks_own_kind",          discordant_setup,                NULL, discordant_attacks_own_kind },
    { "BR54066_discordant_attacks_player",            discordant_setup,                NULL, discordant_attacks_player },

    { "BR45704_captive_is_never_enemy",               captive_setup,                   NULL, captive_is_never_enemy },
    { "BR45704_discord_makes_mutual_enemies",         discordant_ally_pair_setup,      NULL, discord_makes_mutual_enemies },
    { "BR45704_opposite_sides_are_enemies",           opposite_sides_setup,            NULL, opposite_sides_are_enemies },

    { "BR45522_you_and_ally_are_teammates",           ally_setup,                      NULL, you_and_ally_are_teammates },
    { "BR45522_allies_are_teammates",                 ally_pair_setup,                 NULL, allies_are_teammates },
    { "BR45522_followers_of_one_leader_are_teammates", followers_of_one_leader_setup,  NULL, followers_are_teammates },

    { "BR89577_eel_attacks_creature_in_deep_water",   eel_and_water_creature_setup,    NULL, eel_attacks_creature_in_deep_water },
    { "BR89577_eel_ignores_levitating_creature",      eel_and_levitating_creature_setup, NULL, eel_ignores_levitating_creature },

    { "BR11561_monster_avoids_stairs",                stairs_setup,                    NULL, monster_avoids_stairs },
    { "BR11561_player_ignores_stairs",                stairs_setup,                    NULL, player_ignores_stairs },

    { "BR15087_liquid_monster_avoids_dry_land",       liquid_and_dry_land_setup,       NULL, liquid_monster_avoids_dry_land },
    { "BR15087_liquid_monster_accepts_water",         liquid_and_water_setup,          NULL, liquid_monster_accepts_water },

    { "BR50228_hostile_monster_does_not_avoid_player", avoid_player_setup,             NULL, hostile_monster_does_not_avoid_player },

    { "BR27232_monster_avoids_lava",                  lava_setup,                      NULL, monster_avoids_lava },
    { "BR27232_fire_immune_monster_ignores_lava",     fire_immune_lava_setup,          NULL, fire_immune_monster_ignores_lava },

    { "BR51199_beached_liquid_monster_cannot_move",   beached_eel_setup,               NULL, beached_liquid_monster_cannot_move },

    { "BR44948_never_sleeping_monster_starts_awake",  never_sleeping_monster_setup,    NULL, never_sleeping_monster_starts_awake },

    { NULL, NULL, NULL, NULL }
};
