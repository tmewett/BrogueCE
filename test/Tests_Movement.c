// Tests derived from rules/01-movement-environment.md.
//
// Only rules that can be exercised deterministically in the headless test
// harness are converted here; rules about rendering, recordings, and random
// dungeon layout are omitted.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// Positions remembered between setup and verify.
static pos startPos;
static pos platePos;
static pos targetPos;

// ============================================================
// BR31821 — a move is blocked by an obstructing destination square
// ============================================================

static void wall_blocks_setup(void) {
    clearArena();
    setTerrain(posOffset(player.loc, 1, 0), GRANITE);
    startPos = player.loc;
}

static boolean wall_blocks_step(void) {
    moveRight();
    return false;
}

static boolean player_did_not_move(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "player should not have moved");
    return !testFailed;
}

// ============================================================
// BR29427 — a diagonal move is blocked when either flanking square blocks it
// ============================================================

static void corner_blocks_setup(void) {
    clearArena();
    // Obstacles beside the destination, so cutting the corner is not allowed.
    setTerrain(posOffset(player.loc, 0, -1), GRANITE);
    setTerrain(posOffset(player.loc, 1, 0), GRANITE);
    startPos = player.loc;
}

static boolean corner_blocks_step(void) {
    moveUpRight();
    return false;
}

// ============================================================
// BR10367 — a cardinal move is not blocked by corner geometry
// ============================================================

static void cardinal_corner_setup(void) {
    clearArena();
    // All four squares flanking the destination are walls; the destination is floor.
    setTerrain(posOffset(player.loc, 1, -1), GRANITE);
    setTerrain(posOffset(player.loc, 1, 1), GRANITE);
    setTerrain(posOffset(player.loc, 0, -1), GRANITE);
    setTerrain(posOffset(player.loc, 0, 1), GRANITE);
    startPos = player.loc;
    targetPos = posOffset(player.loc, 1, 0);
}

static boolean cardinal_corner_step(void) {
    moveRight();
    return false;
}

static boolean player_moved_right(void) {
    TEST_ASSERT_POS_EQ(player.loc, targetPos, "player should have stepped right");
    return !testFailed;
}

// ============================================================
// BR75019 — stepping onto a square with an item picks it up
// ============================================================

static void floor_item_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    placeItemOnFloor(FOOD, RATION, targetPos);
}

static boolean floor_item_step(void) {
    moveRight();
    return false;
}

static boolean item_was_picked_up(void) {
    TEST_ASSERT(playerHasItem(FOOD, RATION), "stepping on an item should pick it up");
    return !testFailed;
}

// ============================================================
// BR44054 — stepping onto an ally swaps places
// ============================================================

static void ally_swap_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    creature *ally = spawnMonster(MK_RAT, targetPos);
    makeMonsterAlly(ally);
    revealCell(targetPos);
}

static boolean ally_swap_step(void) {
    moveRight();
    return false;
}

static boolean ally_swapped(void) {
    // The player now stands where the ally was, and the ally behind.
    TEST_ASSERT_POS_EQ(player.loc, targetPos, "player should occupy the ally's old square");
    return !testFailed;
}

// ============================================================
// BR96161 — moving into a hostile monster attacks instead
// ============================================================

static void attack_on_bump_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    placeHostileMonster(MK_RAT, targetPos);
    startPos = player.loc;
}

static boolean attack_on_bump_step(void) {
    moveRight();
    return false;
}

static boolean player_stayed_and_monster_hurt(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "bumping a monster should attack, not move");
    return !testFailed;
}

// ============================================================
// BR60519 — you refuse to step into known lava
// ============================================================

static void lava_refusal_setup(void) {
    clearArena();
    setTerrain(posOffset(player.loc, 1, 0), LAVA);
    revealCell(posOffset(player.loc, 1, 0));
    startPos = player.loc;
}

static boolean lava_refusal_step(void) {
    moveRight();
    return false;
}

static boolean refused_lava(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "player should refuse to step into lava");
    TEST_ASSERT(messageWasShown("certain death"), "the player should be warned about lava");
    return !testFailed;
}

// ============================================================
// BR63035 — walking into a chasm asks for confirmation
// ============================================================

static void chasm_prompt_setup(void) {
    clearArena();
    setTerrain(posOffset(player.loc, 1, 0), CHASM);
    revealCell(posOffset(player.loc, 1, 0));
    queueConfirmNo();
    startPos = player.loc;
}

static boolean chasm_prompt_step(void) {
    moveRight();
    return false;
}

static boolean chasm_prompt_refused(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "declining the chasm prompt should cancel the move");
    TEST_ASSERT(messageWasShown("Dive into the depths?"), "the game should ask before diving");
    return !testFailed;
}

// ============================================================
// BR95206 — walking into visible fire asks for confirmation
// ============================================================

static void fire_prompt_setup(void) {
    clearArena();
    setTerrain(posOffset(player.loc, 1, 0), PLAIN_FIRE);
    revealCell(posOffset(player.loc, 1, 0));
    queueConfirmNo();
    startPos = player.loc;
}

static boolean fire_prompt_step(void) {
    moveRight();
    return false;
}

static boolean fire_prompt_refused(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "declining the fire prompt should cancel the move");
    TEST_ASSERT(messageWasShown("Venture into flame?"), "the game should ask before entering fire");
    return !testFailed;
}

// ============================================================
// BR83833 — stepping onto a visible pressure plate asks for confirmation
// ============================================================

static void plate_prompt_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    setTerrain(targetPos, MACHINE_PRESSURE_PLATE);
    revealCell(targetPos);
    queueConfirmNo();
    startPos = player.loc;
}

static boolean plate_prompt_step(void) {
    moveRight();
    return false;
}

static boolean plate_prompt_refused(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "declining the plate prompt should cancel the move");
    TEST_ASSERT(messageWasShown("Step onto the pressure plate?"), "the game should ask before stepping on a plate");
    return !testFailed;
}

// ============================================================
// BR50847 — stepping onto a staircase changes level
// ============================================================

static void stairs_setup(void) {
    clearArena();
    placeDownStairs(posOffset(player.loc, 1, 0));
    startPos = player.loc;
}

static boolean stairs_step(void) {
    moveRight();
    return false;
}

static boolean descended_a_level(void) {
    TEST_ASSERT(!isSameDepth(), "stepping onto the down stairs should descend");
    return !testFailed;
}

// ============================================================
// BR39970 — a creature stepping on a pressure plate depresses it
// ============================================================

static void plate_depress_setup(void) {
    clearArena();
    platePos = posOffset(player.loc, 1, 0);
    setTerrain(platePos, MACHINE_PRESSURE_PLATE);
}

static boolean plate_depress_step(void) {
    moveRight();
    return false;
}

static boolean plate_is_depressed(void) {
    TEST_ASSERT(isTrapTriggered(platePos), "the pressure plate should be depressed");
    return !testFailed;
}

// ============================================================
// BR22348 — a pressure plate resets when nothing remains on it
// ============================================================

static boolean plate_reset_step(void) {
    switch (turnNumber()) {
        case 0: moveRight(); return true;  // step on the plate
        case 1: moveRight(); return true;  // step off it
        default:
            if (turnNumber() < 220) {
                restOneTurn();
                return true;
            }
            return false;
    }
}

static boolean plate_reset(void) {
    TEST_ASSERT(!isTrapTriggered(platePos), "the plate should reset once vacated");
    return !testFailed;
}

// ============================================================
// BR24837 — a creature in lava without protection is killed
// ============================================================

static void lava_kills_setup(void) {
    clearArena();
    setTerrain(player.loc, LAVA);
    revealCell(player.loc);
}

static boolean rest_a_few_turns_step(void) {
    if (turnNumber() < 5 && !rogue.gameHasEnded) {
        restOneTurn();
        return true;
    }
    return false;
}

static boolean player_was_killed(void) {
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "standing in lava should be fatal");
    return !testFailed;
}

// ============================================================
// BR51947 — standing in deep water submerges you
// ============================================================

static void deep_water_setup(void) {
    clearArena();
    setTerrain(player.loc, DEEP_WATER);
    setPlayerPos(player.loc);
}

static boolean deep_water_step(void) {
    restOneTurn();
    return false;
}

static boolean player_is_submerged(void) {
    TEST_ASSERT(isPlayerSubmerged(), "standing in deep water should submerge the player");
    return !testFailed;
}

// ============================================================
// BR91575 — stepping into webbing traps the creature
// ============================================================

static void web_setup(void) {
    clearArena();
    setTerrain(posOffset(player.loc, 1, 0), SPIDERWEB);
}

static boolean web_step(void) {
    moveRight();
    return false;
}

static boolean player_is_webbed(void) {
    TEST_ASSERT(isPlayerStuck(), "stepping into a web should trap the player");
    return !testFailed;
}

// ============================================================
// BR79486 — standing in fire ignites you
// ============================================================

static void standing_in_fire_setup(void) {
    clearArena();
    setTerrain(player.loc, PLAIN_FIRE);
    setPlayerPos(player.loc);
}

static boolean standing_in_fire_step(void) {
    restOneTurn();
    return false;
}

static boolean player_is_burning(void) {
    TEST_ASSERT(isPlayerBurning(), "standing in fire should set the player alight");
    return !testFailed;
}

// ============================================================
// BR34974 — nutrition drops by one each turn
// ============================================================

static int nutritionBefore;

static void nutrition_setup(void) {
    clearArena();
    setPlayerNutrition(2000);
}

static boolean nutrition_step(void) {
    nutritionBefore = playerNutrition();
    restOneTurn();
    return false;
}

static boolean nutrition_dropped(void) {
    TEST_ASSERT_EQ(playerNutrition(), nutritionBefore - 1, "nutrition should drop by one per turn");
    return !testFailed;
}

// ============================================================
// BR95832 — at zero nutrition you lose health each turn
// ============================================================

static int healthBefore;

static void starvation_setup(void) {
    clearArena();
    removeAllFood();
    setPlayerNutrition(0);
    setPlayerHealth(playerMaxHealth());
}

static boolean starvation_step(void) {
    healthBefore = playerHealth();
    restOneTurn();
    return false;
}

static boolean starved(void) {
    TEST_ASSERT(playerHealth() < healthBefore, "starvation should drain hit points");
    return !testFailed;
}

// ============================================================
// BR35928 — a whip strikes a visible enemy up to five squares away
// ============================================================

static creature *whipTarget;
static int whipTargetHealthBefore;

static void whip_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, WHIP, 0);
    equipItemFromPack(WEAPON, WHIP);
    targetPos = posOffset(player.loc, 4, 0);
    whipTarget = placeHostileMonster(MK_RAT, targetPos);
    setMonsterHealth(whipTarget, whipTarget->info.maxHP);
    // Paralyse the target so the strike cannot miss; this test is about reach.
    applyStatusToMonster(whipTarget, STATUS_PARALYZED, 10);
    whipTargetHealthBefore = monsterHealth(whipTarget);
    startPos = player.loc;
}

static boolean whip_step(void) {
    moveRight();
    return false;
}

static boolean whip_hit_distant_enemy(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "the whip should strike without moving the player");
    TEST_ASSERT(monsterHealth(whipTarget) < whipTargetHealthBefore, "the whip should hit an enemy several squares away");
    return !testFailed;
}

// ============================================================
// BR12436 — a spear strikes every enemy in the first two squares
// ============================================================

static creature *spearTarget;
static int spearTargetHealthBefore;

static void spear_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, SPEAR, 0);
    equipItemFromPack(WEAPON, SPEAR);
    targetPos = posOffset(player.loc, 2, 0);
    spearTarget = placeHostileMonster(MK_RAT, targetPos);
    setMonsterHealth(spearTarget, spearTarget->info.maxHP);
    applyStatusToMonster(spearTarget, STATUS_PARALYZED, 10);
    spearTargetHealthBefore = monsterHealth(spearTarget);
}

static boolean spear_step(void) {
    moveRight();
    return false;
}

static boolean spear_hit_distant_enemy(void) {
    TEST_ASSERT(monsterHealth(spearTarget) < spearTargetHealthBefore, "the spear should strike an enemy two squares away");
    return !testFailed;
}

// ============================================================
// BR12763 — a lunging weapon attacks an enemy two squares away
// ============================================================

static creature *lungeTarget;
static int lungeTargetHealthBefore;

static void lunge_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, RAPIER, 0);
    equipItemFromPack(WEAPON, RAPIER);
    targetPos = posOffset(player.loc, 2, 0);
    // A rat would be killed outright, leaving us with a dangling pointer; use a
    // sturdy monster so we can observe the wound after the lunge resolves.
    lungeTarget = placeHostileMonster(MK_TROLL, targetPos);
    setMonsterHealth(lungeTarget, lungeTarget->info.maxHP);
    lungeTargetHealthBefore = monsterHealth(lungeTarget);
    targetPos = posOffset(player.loc, 1, 0);
}

static boolean lunge_step(void) {
    moveRight();
    return false;
}

static boolean lunged_and_moved(void) {
    TEST_ASSERT(monsterHealth(lungeTarget) < lungeTargetHealthBefore, "the rapier should lunge at an enemy two squares away");
    TEST_ASSERT_POS_EQ(player.loc, targetPos, "a lunge completes after the step");
    return !testFailed;
}

// ============================================================

const testCase tests_movement[] = {
    { "BR31821_move_blocked_by_obstruction",   wall_blocks_setup,      wall_blocks_step,         player_did_not_move },
    { "BR29427_diagonal_blocked_by_corner",    corner_blocks_setup,    corner_blocks_step,       player_did_not_move },
    { "BR10367_cardinal_not_corner_blocked",   cardinal_corner_setup,  cardinal_corner_step,     player_moved_right },
    { "BR75019_step_on_item_picks_it_up",      floor_item_setup,       floor_item_step,          item_was_picked_up },
    { "BR44054_step_onto_ally_swaps_places",   ally_swap_setup,        ally_swap_step,           ally_swapped },
    { "BR96161_move_into_hostile_attacks",     attack_on_bump_setup,   attack_on_bump_step,      player_stayed_and_monster_hurt },
    { "BR60519_refuse_known_lava",             lava_refusal_setup,     lava_refusal_step,        refused_lava },
    { "BR63035_chasm_prompt_cancels_move",     chasm_prompt_setup,     chasm_prompt_step,        chasm_prompt_refused },
    { "BR95206_fire_prompt_cancels_move",      fire_prompt_setup,      fire_prompt_step,         fire_prompt_refused },
    { "BR83833_pressure_plate_prompt_cancels", plate_prompt_setup,     plate_prompt_step,        plate_prompt_refused },
    { "BR50847_stairs_change_level",           stairs_setup,           stairs_step,              descended_a_level },
    { "BR39970_pressure_plate_depresses",      plate_depress_setup,    plate_depress_step,       plate_is_depressed },
    { "BR22348_pressure_plate_resets",         plate_depress_setup,    plate_reset_step,         plate_reset },
    { "BR24837_lava_kills",                    lava_kills_setup,       rest_a_few_turns_step,    player_was_killed },
    { "BR51947_deep_water_submerges",          deep_water_setup,       deep_water_step,          player_is_submerged },
    { "BR91575_web_traps_creature",            web_setup,              web_step,                 player_is_webbed },
    { "BR79486_standing_in_fire_ignites",      standing_in_fire_setup, standing_in_fire_step,    player_is_burning },
    { "BR34974_nutrition_drops_each_turn",     nutrition_setup,        nutrition_step,           nutrition_dropped },
    { "BR95832_starvation_drains_health",      starvation_setup,       starvation_step,          starved },
    { "BR35928_whip_strikes_distant_enemy",    whip_setup,             whip_step,                whip_hit_distant_enemy },
    { "BR12436_spear_strikes_two_squares",     spear_setup,            spear_step,               spear_hit_distant_enemy },
    { "BR12763_rapier_lunges_two_squares",     lunge_setup,            lunge_step,               lunged_and_moved },

    { NULL, NULL, NULL, NULL }
};
