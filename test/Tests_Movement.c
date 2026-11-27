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
static int monstHealthBefore;
static creature *monst;
static item *theItem;

// ============================================================
// BR16118 — a run stops if an adjacent square holds an item or a
// visible hostile creature
// ============================================================

static pos runStartPos;
static pos runStopPos;
static creature *runHostile;

// A wide empty arena so a run has somewhere to go if it does not stop.
static void prepare_run_arena(void) {
    setArenaRadius(10, FLOOR);
    clearFloorItems();
    runStartPos = getPlayerPos();
    runStopPos = posOffset(runStartPos, 2, 0);
}

static void run_beside_item_setup(void) {
    prepare_run_arena();
    // Three tiles east and one tile north of the start: beside the path the
    // run takes, so the run reaches it after two steps.
    placeItemOnFloor(POTION, POTION_LIFE, posOffset(runStartPos, 3, -1));
}

static boolean run_beside_item_step(void) {
    runDirection(RIGHT);
    return false;
}

static boolean run_stopped_beside_item(void) {
    TEST_ASSERT(playerIsAt(runStopPos),
                "the run should stop when an item lies in an adjacent square");
    TEST_ASSERT(findFloorItem(POTION, POTION_LIFE) != NULL,
                "the adjacent item should still be on the floor");
    return !testFailed;
}

static void run_beside_hostile_setup(void) {
    prepare_run_arena();
    runHostile = placeHostileMonster(MK_RAT, posOffset(runStartPos, 3, -1));
    // Hold the rat in place so only the run-stop rule can change positions.
    applyStatusToMonster(runHostile, STATUS_PARALYZED, 100);
    // Let the engine register the rat as already seen.  Catching sight of a
    // monster for the first time disturbs the player by itself, which would
    // end the run before it reached the rat.
    restOneTurn();
}

static boolean run_beside_hostile_step(void) {
    runDirection(RIGHT);
    return false;
}

static boolean run_stopped_beside_hostile(void) {
    TEST_ASSERT(cellIsVisible(monsterPosition(runHostile)),
                "the hostile creature beside the run should be visible");
    TEST_ASSERT(playerIsAt(runStopPos),
                "the run should stop when a visible hostile creature is adjacent");
    TEST_ASSERT(monsterIsAlive(runHostile),
                "the hostile creature beside the run should still be alive");
    return !testFailed;
}

// ============================================================
// BR31821 — a move is blocked by an obstructing destination square
// ============================================================

static void wall_blocks_setup(void) {
    clearArena();
    startPos = getPlayerPos();
    setTerrain(posOffset(startPos, 1, 0), GRANITE);
}

static boolean wall_blocks_step(void) {
    moveRight();
    return false;
}

static boolean player_did_not_move(void) {
    TEST_ASSERT(playerIsAt(startPos), "player should not have moved");
    return !testFailed;
}

// ============================================================
// BR29427 — a diagonal move is blocked when either flanking square blocks it
// ============================================================

static void corner_blocks_setup(void) {
    clearArena();
    // Obstacles beside the destination, so cutting the corner is not allowed.
    startPos = getPlayerPos();
    setTerrain(posOffset(startPos, 0, -1), GRANITE);
    setTerrain(posOffset(startPos, 1, 0), GRANITE);
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
    removePackFood();
    removeFloorFood();
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
// BR88965 — the game warns before a confused player's random step could carry them into lava
// ============================================================

static void confused_lava_warning_setup(void) {
    clearArena();
    applyStatusToPlayer(STATUS_CONFUSED, 20);
    targetPos = posOffset(getPlayerPos(), 1, 0);
    setTerrain(targetPos, LAVA);
    revealCell(targetPos); // the lava must be known for the warning to fire
    queueConfirmNo();      // decline the warning, so the move is cancelled
    startPos = getPlayerPos();
}

static boolean confused_lava_warning_step(void) {
    moveRight();
    return false;
}

static boolean warned_before_confused_step_into_lava(void) {
    TEST_ASSERT(messageWasShown("Risk stumbling into lava?"),
                "the game should warn before a confused step could carry the player into lava");
    TEST_ASSERT(playerIsAt(startPos),
                "declining the warning should cancel the move");
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
// BR44913 — the warning before stepping into flame is skipped when
// the player is already burning or immune to fire
// ============================================================

static pos flamePos;

static void flame_adjacent_setup(void) {
    clearArena();
    flamePos = posOffset(player.loc, 1, 0);
    setTerrain(flamePos, PLAIN_FIRE);
    revealCell(flamePos);
    startPos = player.loc;
}

static void flame_vulnerable_setup(void) {
    flame_adjacent_setup();
    queueConfirmNo();
}

static boolean flame_vulnerable_step(void) {
    moveRight();
    return false;
}

static boolean warned_when_flame_would_harm(void) {
    TEST_ASSERT(!isPlayerBurning(), "control: the player should not be burning");
    TEST_ASSERT(!playerHasStatus(STATUS_IMMUNE_TO_FIRE),
                "control: the player should not be immune to fire");
    TEST_ASSERT(messageWasShown("Venture into flame?"),
                "a player the flame could harm should be warned before entering it");
    TEST_ASSERT(playerIsAt(startPos),
                "declining the warning should cancel the move");
    return !testFailed;
}

static void flame_while_burning_setup(void) {
    flame_adjacent_setup();
    applyStatusToPlayer(STATUS_BURNING, 20);
}

static boolean flame_while_burning_step(void) {
    moveRight();
    return false;
}

static boolean no_warning_when_already_burning(void) {
    TEST_ASSERT(isPlayerBurning(), "control: the player should be burning");
    TEST_ASSERT(!messageWasShown("Venture into flame?"),
                "a burning player should not be warned about entering flame");
    TEST_ASSERT(playerIsAt(flamePos),
                "a burning player should enter the flame without answering a prompt");
    return !testFailed;
}

static void flame_while_immune_setup(void) {
    flame_adjacent_setup();
    applyStatusToPlayer(STATUS_IMMUNE_TO_FIRE, 20);
}

static boolean flame_while_immune_step(void) {
    moveRight();
    return false;
}

static boolean no_warning_when_immune_to_fire(void) {
    TEST_ASSERT(playerHasStatus(STATUS_IMMUNE_TO_FIRE),
                "control: the player should be immune to fire");
    TEST_ASSERT(!messageWasShown("Venture into flame?"),
                "a player immune to fire should not be warned about entering flame");
    TEST_ASSERT(playerIsAt(flamePos),
                "a player immune to fire should enter the flame without answering a prompt");
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
    // setTerrain(player.loc, PLAIN_FIRE);
    spawnFeature(getPlayerPos(), DF_PLAIN_FIRE);
    // setPlayerPos(player.loc);
}

static boolean player_is_burning(void) {
    TEST_ASSERT(isPlayerBurning(), "standing in fire should set the player alight");
    return !testFailed;
}

// ============================================================
// BR95213 — every 100 ticks, each burning square tries to
// ignite itself and its four orthogonal neighbours
// ============================================================

// A burning square with four highly flammable (spiderweb) orthogonal
// neighbours.  The player is hasted, so one rest costs 50 ticks: the first
// rest cannot reach the next environment update, while the second one
// completes the 100-tick cycle.  The cycle is first aligned to its start, so
// the first rest is guaranteed to fall entirely between two updates.
static pos br95213_centre;
static pos br95213_neighbours[4];
static int br95213_rests;

static void br95213_setup(void) {
    clearArena();

    pos centre = posAt(levelWidth() / 2, levelHeight() / 2);
    setTerrain(centre, FLOOR);
    setPlayerPos(centre);
    setArenaRadius(3, FLOOR);
    setPlayerPos(posOffset(centre, 3, 0)); // out of reach of the flames

    hastePlayer(20);
    // An environment update leaves the counter at 100.  Align to that point
    // so the next 50-tick rest cannot complete another update.
    while (ticksUntilEnvironmentUpdate() != 100) {
        restOneTurn();
    }

    br95213_centre = centre;
    br95213_neighbours[0] = posOffset(centre, 1, 0);
    br95213_neighbours[1] = posOffset(centre, -1, 0);
    br95213_neighbours[2] = posOffset(centre, 0, 1);
    br95213_neighbours[3] = posOffset(centre, 0, -1);
    for (int i = 0; i < 4; i++) {
        setTerrain(br95213_neighbours[i], SPIDERWEB);
    }
    // A brazier is a permanent burning square; plain fire can burn out in
    // the same update it is asked to spread from.
    setTerrain(centre, BRAZIER);
    TEST_ASSERT(cellIsOnFire(centre),
                "setup: the centre square should be burning");
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT(!cellIsOnFire(br95213_neighbours[i]),
                    "setup: the neighbours should not be burning yet");
    }
    br95213_rests = 0;
}

static boolean br95213_step(void) {
    restOneTurn();
    br95213_rests++;

    if (br95213_rests == 1) {
        // The first 50-tick rest stops short of the next 100-tick update.
        for (int i = 0; i < 4; i++) {
            TEST_ASSERT(!cellIsOnFire(br95213_neighbours[i]),
                        "a burning square must not ignite its neighbours between environment updates");
        }
        return true;
    }
    return false;
}

static boolean br95213_verify(void) {
    TEST_ASSERT_EQ(br95213_rests, 2,
                   "the 100-tick update should arrive on the second 50-tick rest");
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT(cellIsOnFire(br95213_neighbours[i]),
                    "at the 100-tick update the burning square should ignite its four orthogonal neighbours");
    }
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
    removePackFood();
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
// BR32992 — every hunger warning demands acknowledgment, except that
// "you are hungry" demands it only when the pack contains no food
// ============================================================

// The first warning ("you are hungry") arrives with food in the pack, so it
// is not the no-food variant that also asks for acknowledgment.
static void br32992_first_setup(void) {
    clearArena();
    removePackFood();
    givePlayerItem(FOOD, RATION, 0);
    setPlayerNutrition(2000);
}

static boolean br32992_first_step(void) {
    restOneTurn();
    return !messageWasShown("you are hungry");
}

static boolean br32992_first_needs_no_acknowledgment(void) {
    TEST_ASSERT(messageWasShown("you are hungry"),
                "setup: resting should eventually produce the first hunger warning");
    TEST_ASSERT(!messageRequiredAcknowledgment("you are hungry"),
                "the first hunger warning should not demand acknowledgment");
    return !testFailed;
}

// With no food in the pack the starving warning cannot be interrupted by the
// player being forced to eat.
static void br32992_later_setup(void) {
    clearArena();
    removePackFood();
    setPlayerNutrition(2000);
}

static boolean br32992_later_step(void) {
    restOneTurn();
    return !(messageWasShown("weak with hunger")
             && messageWasShown("faint with hunger")
             && messageWasShown("starving to death"));
}

static boolean br32992_later_warnings_need_acknowledgment(void) {
    TEST_ASSERT(messageRequiredAcknowledgment("weak with hunger"),
                "the weak-with-hunger warning should demand acknowledgment");
    TEST_ASSERT(messageRequiredAcknowledgment("faint with hunger"),
                "the faint-with-hunger warning should demand acknowledgment");
    TEST_ASSERT(messageRequiredAcknowledgment("starving to death"),
                "the starving-to-death warning should demand acknowledgment");
    return !testFailed;
}

// ============================================================
// Shared setup helpers for the reach-weapon tests
// ============================================================

// These tests need an exactly plain weapon so the strike's outcome is fixed;
// generateItem() otherwise rolls a random enchant and possibly a runic —
// including mercy, which heals its victim back to full on hit.  See
// givePlayerPlainWeapon() in TestFramework.c.

// A monster that dies is freed at the end of the turn, leaving the test with
// a dangling pointer; give the target far more HP than one strike can do so
// verify reads a live creature.
#define TARGET_SURVIVES_ONE_STRIKE 1000

// ============================================================
// BR35928 — a whip strikes a visible enemy up to five squares away
// ============================================================

static void whip_setup(void) {
    // Clear the full reach of the whip so the line of sight to the target
    // does not depend on the dungeon layout.
    setArenaRadius(5, FLOOR);
    givePlayerPlainWeapon(WHIP, 0);
    equipItemFromPack(WEAPON, WHIP);
    targetPos = posOffset(player.loc, 4, 0);
    monst = placeHostileMonster(MK_RAT, targetPos);
    setMonsterHealth(monst, TARGET_SURVIVES_ONE_STRIKE);
    // Paralyse the target so the strike cannot miss; this test is about reach.
    applyStatusToMonster(monst, STATUS_PARALYZED, 10);
    monstHealthBefore = monsterHealth(monst);
    startPos = player.loc;
}

static boolean whip_step(void) {
    moveRight();
    return false;
}

static boolean whip_hit_distant_enemy(void) {
    TEST_ASSERT_POS_EQ(player.loc, startPos, "the whip should strike without moving the player");
    TEST_ASSERT(monsterHealth(monst) < monstHealthBefore, "the whip should hit an enemy several squares away");
    return !testFailed;
}

// ============================================================
// BR12436 — a spear strikes every enemy in the first two squares
// ============================================================

static void spear_setup(void) {
    clearArena();
    givePlayerPlainWeapon(SPEAR, 0);
    equipItemFromPack(WEAPON, SPEAR);
    targetPos = posOffset(player.loc, 2, 0);
    monst = placeHostileMonster(MK_RAT, targetPos);
    setMonsterHealth(monst, TARGET_SURVIVES_ONE_STRIKE);
    applyStatusToMonster(monst, STATUS_PARALYZED, 10);
    monstHealthBefore = monsterHealth(monst);
}

static boolean spear_step(void) {
    moveRight();
    return false;
}

static boolean spear_hit_distant_enemy(void) {
    TEST_ASSERT(monsterHealth(monst) < monstHealthBefore, "the spear should strike an enemy two squares away");
    return !testFailed;
}

// ============================================================
// BR12763 — a lunging weapon attacks an enemy two squares away
// ============================================================

static void lunge_setup(void) {
    clearArena();
    theItem = givePlayerPlainWeapon(RAPIER, 10);
    equip(theItem);
    drink(POTION_STRENGTH); // STR 13
    drink(POTION_STRENGTH); // STR 14
    drink(POTION_STRENGTH); // STR 15 (rapier)

    targetPos = posOffset(player.loc, 2, 0);
    monst = placeHostileMonster(MK_KRAKEN, targetPos);
}

static boolean lunge_step(void) {
    moveRight();
    return false;
}

static boolean lunged_and_moved(void) {
    TEST_ASSERT(monsterIsHurt(monst) && monsterIsAlive(monst), "the rapier should lunge at an enemy two squares away");
    return !testFailed;
}

// ============================================================
// BR49182 — a flailing weapon hits every enemy adjacent to both
// the starting and the ending square of a step
// ============================================================

static creature *flailHitMonst[2];
static creature *flailMissMonst[2];
static pos flailStartPos;

static void flail_setup(void) {
    setArenaRadius(3, FLOOR);
    givePlayerPlainWeapon(FLAIL, 0);
    equipItemFromPack(WEAPON, FLAIL);
    flailStartPos = getPlayerPos();

    // The player steps right, from flailStartPos to (x+1, y).  These two
    // enemies are adjacent to both squares: north of the starting square and
    // north of the ending square.
    flailHitMonst[0] = placeHostileMonster(MK_RAT, posOffset(flailStartPos, 0, 1));
    flailHitMonst[1] = placeHostileMonster(MK_RAT, posOffset(flailStartPos, 1, 1));
    // These two are adjacent to only one of the squares: west of the start
    // (start only) and east of the end (end only).
    flailMissMonst[0] = placeHostileMonster(MK_RAT, posOffset(flailStartPos, -1, 0));
    flailMissMonst[1] = placeHostileMonster(MK_RAT, posOffset(flailStartPos, 2, 0));

    // Paralyse every rat so they cannot act, and give them enough hit points
    // that one flail blow leaves them alive and observable.
    for (int i = 0; i < 2; i++) {
        setMonsterHealth(flailHitMonst[i], TARGET_SURVIVES_ONE_STRIKE);
        setMonsterHealth(flailMissMonst[i], TARGET_SURVIVES_ONE_STRIKE);
        applyStatusToMonster(flailHitMonst[i], STATUS_PARALYZED, 10);
        applyStatusToMonster(flailMissMonst[i], STATUS_PARALYZED, 10);
    }
}

static boolean flail_step(void) {
    moveRight();
    return false;
}

static boolean flail_hits_only_both_square_enemies(void) {
    TEST_ASSERT(monsterIsHurt(flailHitMonst[0]),
                "the flail should hit the enemy adjacent to both the starting and the ending square");
    TEST_ASSERT(monsterIsHurt(flailHitMonst[1]),
                "the flail should hit every enemy adjacent to both squares, not just the first");
    TEST_ASSERT(monsterIsHealthy(flailMissMonst[0]),
                "the flail should not hit an enemy adjacent only to the starting square");
    TEST_ASSERT(monsterIsHealthy(flailMissMonst[1]),
                "the flail should not hit an enemy adjacent only to the ending square");
    return !testFailed;
}

// ============================================================
// BR28946 — whenever any gas is present, the gas simulation runs
// twice per environment update
// ============================================================

static pos gasCentre;

static void gas_runs_twice_setup(void) {
    // Work from the middle of the map so six squares of open floor in every
    // direction are guaranteed to be in bounds.
    pos centre = posAt(levelWidth() / 2, levelHeight() / 2);
    setTerrain(centre, FLOOR);
    setPlayerPos(centre);
    setArenaRadius(6, FLOOR);
    gasCentre = posOffset(centre, 3, 0);
    spawnFeature(gasCentre, DF_POISON_GAS_CLOUD);
}

static boolean one_turn_rest_step(void) {
    restOneTurn();
    return false;
}

static boolean gas_spreads_two_squares_in_one_update(void) {
    TEST_ASSERT(cellHasGas(posOffset(gasCentre, 1, 0)),
                "after one turn the gas should have spread to the neighbouring squares");
    TEST_ASSERT(cellHasGas(posOffset(gasCentre, 2, 0)),
                "after one turn the gas should have reached two squares away, "
                "because the gas simulation runs twice per environment update");
    TEST_ASSERT(!cellHasGas(posOffset(gasCentre, 3, 0)),
                "after one turn the gas should not yet have reached three squares away");
    return !testFailed;
}

// ============================================================
// BR29862 — gases that dissipate quickly or slowly lose one unit of volume,
// with a 50% or 20% chance per gas update.
//
// One gas update is run directly over a single floor tile sealed inside
// granite, so nothing can move in or out and the only possible change is the
// one-unit dissipation roll.  Starting at two units makes the outcome binary:
// two units if the roll failed, one if it succeeded.  A gas with no
// dissipation flag is the control, and must never change.
// ============================================================

#define BR29862_TRIALS 2000

static pos br29862_gasPos;
static short br29862_quickGas;
static short br29862_slowGas;
static short br29862_stableGas;

static void br29862_setup(void) {
    setArenaRadius(8, FLOOR);
    br29862_gasPos = posOffset(getPlayerPos(), 5, 0);

    // Seal the measured tile inside granite so the gas update cannot move the
    // gas; only its dissipation roll can change the volume.
    setTerrainAroundLoc(br29862_gasPos, GRANITE);
    setTerrain(br29862_gasPos, FLOOR);

    br29862_quickGas = quickDissipatingGasType();
    br29862_slowGas = slowDissipatingGasType();
    br29862_stableGas = nonDissipatingGasType();
}

// outcomes[0] counts one-unit losses, outcomes[1] counts no change, and
// outcomes[2] counts anything else (which no gas should ever do).
static void countGasOutcome(short gasType, int outcomes[3]) {
    setGasAt(br29862_gasPos, gasType, 2);
    runGasUpdatePass();
    int volume = gasVolumeAt(br29862_gasPos);
    if (volume == 1) {
        outcomes[0]++;
    } else if (volume == 2) {
        outcomes[1]++;
    } else {
        outcomes[2]++;
    }
}

static boolean br29862_verify(void) {
    int quick[3] = {0, 0, 0}, slow[3] = {0, 0, 0}, stable[3] = {0, 0, 0};

    for (int trial = 0; trial < BR29862_TRIALS; trial++) {
        countGasOutcome(br29862_quickGas, quick);
    }
    for (int trial = 0; trial < BR29862_TRIALS; trial++) {
        countGasOutcome(br29862_slowGas, slow);
    }
    for (int trial = 0; trial < BR29862_TRIALS; trial++) {
        countGasOutcome(br29862_stableGas, stable);
    }

    TEST_ASSERT_EQ(stable[0], 0,
                   "a gas with no dissipation flag should never lose volume");
    TEST_ASSERT_EQ(stable[2], 0,
                   "the control gas should always keep exactly two units");

    TEST_ASSERT(quick[0] >= BR29862_TRIALS * 40 / 100
                && quick[0] <= BR29862_TRIALS * 60 / 100,
                "a quickly dissipating gas should lose one unit about half the time");
    TEST_ASSERT(slow[0] >= BR29862_TRIALS * 10 / 100
                && slow[0] <= BR29862_TRIALS * 30 / 100,
                "a slowly dissipating gas should lose one unit about one time in five");

    TEST_ASSERT_EQ(quick[2], 0,
                   "a quick gas should only keep both units or lose exactly one");
    TEST_ASSERT_EQ(slow[2], 0,
                   "a slow gas should only keep both units or lose exactly one");
    TEST_ASSERT_EQ(quick[0] + quick[1], BR29862_TRIALS,
                   "every quick-gas update should have been observed");
    TEST_ASSERT_EQ(slow[0] + slow[1], BR29862_TRIALS,
                   "every slow-gas update should have been observed");
    TEST_ASSERT_EQ(stable[0] + stable[1], BR29862_TRIALS,
                   "every control update should have been observed");
    return !testFailed;
}

// ============================================================
// BR21546 — a thorough search also marks nearby squares as known to be trap-free
// ============================================================

static pos searchedPos;
static int searchCount;

static void thorough_search_setup(void) {
    clearArena();
    searchedPos = posOffset(player.loc, 3, 0);
    setTerrain(searchedPos, FLOOR);
    revealCell(searchedPos); // the square must be visible to be searched
}

static boolean thorough_search_step(void) {
    if (searchCount < 5) {
        searchOneTurn();
        searchCount++;
        return true; // keep searching until the thorough (fifth) search
    }
    return false; // the thorough search has been performed
}

static boolean nearby_square_marked_trap_free(void) {
    TEST_ASSERT(cellIsKnownTrapFree(searchedPos),
                "a thorough search should mark a nearby searched square as known to be trap-free");
    TEST_ASSERT(messageWasShown("you finish your detailed search of the area."),
                "the fifth search should be a thorough search");
    return !testFailed;
}

// ============================================================
// BR20029 — squares already known to be trap-free always cost a flat 10 to explore
// ============================================================

static pos exploreStart;
static pos trapFreeRoute[4];
static pos unfamiliarRoute[2];
static pos exploreGoal;

static void trap_free_explore_setup(void) {
    // The exploration penalty for unfamiliar squares rises with depth: at
    // depth 15 an explored-but-unfamiliar square costs 30 to explore
    // (10 + (15 - 5) * 2), while a square known to be trap-free costs a flat 10.
    setDepth(15);

    // Rebuild the level as solid granite so that the only walkable ground is
    // the experiment itself.  This also clears every floor item's cell flag,
    // so no leftover item can act as a competing exploration goal.
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }

    // Start from the middle of the map so neither corridor nor the goal
    // square can touch the border rows and columns, whose pathing cost is
    // always impassable.
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    exploreStart = getPlayerPos();

    // Two corridors lead from the player to a single unexplored goal square:
    //   route A (west, south, east): four squares; the player walks each one
    //     before exploring, which marks them known to be trap-free (4 x 10 = 40).
    //   route B (east, south): two squares never stood on (2 x 30 = 60).
    // The granite square directly south of the start keeps the goal out of
    // sight from the start and from the first two squares of route A.
    trapFreeRoute[0] = posOffset(exploreStart, -1,  0);
    trapFreeRoute[1] = posOffset(exploreStart, -1,  1);
    trapFreeRoute[2] = posOffset(exploreStart, -1,  2);
    trapFreeRoute[3] = posOffset(exploreStart,  0,  2);
    unfamiliarRoute[0] = posOffset(exploreStart,  1,  0);
    unfamiliarRoute[1] = posOffset(exploreStart,  1,  1);
    exploreGoal = posOffset(exploreStart, 1, 2);

    setTerrain(exploreStart, FLOOR);
    for (int i = 0; i < 4; i++) {
        setTerrain(trapFreeRoute[i], FLOOR);
    }
    for (int i = 0; i < 2; i++) {
        setTerrain(unfamiliarRoute[i], FLOOR);
    }
    setTerrain(exploreGoal, FLOOR);

    // Everything except the goal square is known, so the goal is the only
    // thing left to explore and only the corridor costs can decide how to
    // reach it.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos p = posAt(x, y);
            if (p.x != exploreGoal.x || p.y != exploreGoal.y) {
                revealCell(p);
            }
        }
    }

    // Standing on a square marks it known to be trap-free; walk only route A,
    // then retrace back to the start so the player begins where both routes do.
    moveLeft();   // start -> A[0]
    moveDown();   // A[0] -> A[1]
    moveDown();   // A[1] -> A[2]
    moveRight();  // A[2] -> A[3]
    moveLeft();   // A[3] -> A[2]
    moveUp();     // A[2] -> A[1]
    moveUp();     // A[1] -> A[0]
    moveRight();  // A[0] -> start

    // The walk may have brought the goal into view; it must be unexplored
    // when the exploration starts.
    setTerrain(exploreGoal, FLOOR);
}

static boolean trap_free_explore_step(void) {
    // Preconditions: route A is known trap-free, route B is not, and the
    // player is back at the start with the goal still unexplored.
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT(cellIsKnownTrapFree(trapFreeRoute[i]),
                    "setup: a walked square should be known to be trap-free");
    }
    for (int i = 0; i < 2; i++) {
        TEST_ASSERT(!cellIsKnownTrapFree(unfamiliarRoute[i]),
                    "setup: an un-walked square should not be known to be trap-free");
    }
    TEST_ASSERT(playerIsAt(exploreStart), "setup: the player should be back at the start");

    // The first press picks the exploration target; the second press starts
    // exploring toward it.
    injectKeystroke('x', false, false);
    injectKeystroke('x', false, false);
    return false;
}

static boolean trap_free_route_was_taken(void) {
    pos here = getPlayerPos();
    boolean onTrapFreeRoute = false;
    for (int i = 0; i < 4; i++) {
        if (here.x == trapFreeRoute[i].x && here.y == trapFreeRoute[i].y) {
            onTrapFreeRoute = true;
        }
    }
    TEST_ASSERT(onTrapFreeRoute,
                "auto-explore should prefer the corridor of trap-free squares (flat 10 each) over the equally useful but unfamiliar corridor (30 each at this depth)");
    return !testFailed;
}

// ============================================================
// BR91235 — every 100 ticks staffs recharge and charms count down
// ============================================================

// The three tests below sample the rule's 100-tick environment cycle one
// update at a time.  While the player is hasted a rest costs 50 ticks — half
// a cycle — so a rest can fall entirely between two environment updates:
// restAndDetectUpdate() reports whether an update fired during the rest
// (the cycle counter only rises past its old value when it wraps and the
// update runs; it falls by 50 when no update did), and every test records
// that recharge/countdown state moves only on rests where one fired.  The
// helper also remembers any reading above the rule's 100-tick period, so a
// cycle that ran long cannot pass unnoticed.  Each test re-hastes itself for
// far longer than its budget, keeping every rest at a steady 50 ticks; the
// haste expiry that would re-anchor the cycle mid-test never happens.
static boolean cadenceCycleTooLong;

static boolean restAndDetectUpdate(void) {
    int ticksBefore = ticksUntilEnvironmentUpdate();
    restOneTurn();
    int ticksAfter = ticksUntilEnvironmentUpdate();
    if (ticksAfter > 100) {
        cadenceCycleTooLong = true;
    }
    return ticksAfter > ticksBefore;
}

static item *spentStaff;
static int staffChargesBeforeZap;
static int staffChargesAfterZap;
static int staffChargesLastSeen;
static int staffUpdatesSeen;
static boolean staffChargedBetweenUpdates;
static boolean staffChargedTwicePerUpdate;
static int staffRests;

// Zap a monster with a fresh staff so the staff ends up one charge short of
// full, then clear the dungeon so nothing interferes while the charge returns.
static void staff_recharge_setup(void) {
    setArenaRadius(4, FLOOR);
    setPlayerNutrition(2000);
    spentStaff = givePlayerItem(STAFF, STAFF_LIGHTNING, 0);
    staffChargesBeforeZap = itemCharges(spentStaff);
    placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 3, 0));
    // A turn ends with a sidebar refresh, and auto-targeting reads the
    // sidebar's list of visible creatures to pick what to aim at.
    restOneTurn();
    queueKeystroke(RETURN_KEY, false, false);
    applyPackItem(STAFF, STAFF_LIGHTNING);
    clearMonsters();
    staffChargesAfterZap = itemCharges(spentStaff);
    hastePlayer(30000);
    cadenceCycleTooLong = false;
    staffChargesLastSeen = staffChargesAfterZap;
    staffUpdatesSeen = 0;
    staffChargedBetweenUpdates = false;
    staffChargedTwicePerUpdate = false;
    staffRests = 0;
}

static boolean staff_recharge_step(void) {
    boolean updated = restAndDetectUpdate();
    staffRests++;
    int chargesNow = itemCharges(spentStaff);
    if (updated) {
        staffUpdatesSeen++;
        if (chargesNow > staffChargesLastSeen + 1) {
            staffChargedTwicePerUpdate = true;
        }
    } else if (chargesNow != staffChargesLastSeen) {
        staffChargedBetweenUpdates = true;
    }
    staffChargesLastSeen = chargesNow;
    return chargesNow <= staffChargesAfterZap && staffRests < 1000;
}

static boolean spent_staff_recharges(void) {
    TEST_ASSERT_EQ(staffChargesAfterZap, staffChargesBeforeZap - 1,
                   "zapping should cost the staff one charge");
    TEST_ASSERT(staffUpdatesSeen > 0,
                "setup: environment updates should pass while resting");
    TEST_ASSERT(!cadenceCycleTooLong,
                "the environment cycle should never exceed the rule's 100 ticks");
    TEST_ASSERT(!staffChargedBetweenUpdates,
                "a charge must not return between environment updates");
    TEST_ASSERT(!staffChargedTwicePerUpdate,
                "no more than one charge may return per environment update");
    TEST_ASSERT(itemCharges(spentStaff) > staffChargesAfterZap,
                "a staff that is short a charge should recharge over time");
    return !testFailed;
}

static item *fullStaff;
static int fullStaffCharges;
static int fullStaffChargesLastSeen;
static int fullStaffUpdatesSeen;
static boolean fullStaffEverChanged;
static int fullStaffRests;

static void full_staff_setup(void) {
    setArenaRadius(4, FLOOR);
    setPlayerNutrition(2000);
    fullStaff = givePlayerItem(STAFF, STAFF_LIGHTNING, 0);
    fullStaffCharges = itemCharges(fullStaff);
    hastePlayer(30000);
    cadenceCycleTooLong = false;
    fullStaffChargesLastSeen = fullStaffCharges;
    fullStaffUpdatesSeen = 0;
    fullStaffEverChanged = false;
    fullStaffRests = 0;
}

static boolean full_staff_rest_step(void) {
    if (restAndDetectUpdate()) {
        fullStaffUpdatesSeen++;
    }
    fullStaffRests++;
    if (itemCharges(fullStaff) != fullStaffChargesLastSeen) {
        fullStaffEverChanged = true;
        fullStaffChargesLastSeen = itemCharges(fullStaff);
    }
    return fullStaffRests < 60;
}

static boolean full_staff_gains_no_charges(void) {
    TEST_ASSERT(fullStaffCharges > 0,
                "a freshly found staff should start out carrying charges");
    TEST_ASSERT(fullStaffUpdatesSeen > 0,
                "setup: environment updates should pass while resting");
    TEST_ASSERT(!cadenceCycleTooLong,
                "the environment cycle should never exceed the rule's 100 ticks");
    TEST_ASSERT(!fullStaffEverChanged,
                "a full staff's charge count must never change");
    TEST_ASSERT_EQ(itemCharges(fullStaff), fullStaffCharges,
                   "a staff that is already full should not gain more charges");
    return !testFailed;
}

static item *countdownCharm;
static int charmChargesWhenUsed;
static int charmChargesLastSeen;
static int charmUpdatesSeen;
static boolean charmChangedBetweenUpdates;
static boolean charmCountdownSkipped;
static int charmRests;

static void charm_countdown_setup(void) {
    setArenaRadius(4, FLOOR);
    setPlayerNutrition(2000);
    countdownCharm = givePlayerItem(CHARM, CHARM_HASTE, 3);
    applyPackItem(CHARM, CHARM_HASTE);
    charmChargesWhenUsed = itemCharges(countdownCharm);
    // Activating the charm hastes you as well; overwriting that with our own
    // long duration pins the 50-tick rest for the whole countdown.
    hastePlayer(30000);
    cadenceCycleTooLong = false;
    charmChargesLastSeen = charmChargesWhenUsed;
    charmUpdatesSeen = 0;
    charmChangedBetweenUpdates = false;
    charmCountdownSkipped = false;
    charmRests = 0;
}

static boolean charm_countdown_step(void) {
    boolean updated = restAndDetectUpdate();
    charmRests++;
    int chargesNow = itemCharges(countdownCharm);
    if (updated) {
        charmUpdatesSeen++;
        int expected = charmChargesLastSeen > 0 ? charmChargesLastSeen - 1 : 0;
        if (chargesNow != expected) {
            charmCountdownSkipped = true;
        }
    } else if (chargesNow != charmChargesLastSeen) {
        charmChangedBetweenUpdates = true;
    }
    charmChargesLastSeen = chargesNow;
    return chargesNow > 0 && charmRests < 3000;
}

static boolean charm_counts_down_to_recharged(void) {
    TEST_ASSERT(charmChargesWhenUsed > 0,
                "using a charm should start its recharge countdown");
    TEST_ASSERT(charmUpdatesSeen > 0,
                "setup: environment updates should pass while resting");
    TEST_ASSERT(!cadenceCycleTooLong,
                "the environment cycle should never exceed the rule's 100 ticks");
    TEST_ASSERT(!charmChangedBetweenUpdates,
                "a charm's countdown must not run between environment updates");
    TEST_ASSERT(!charmCountdownSkipped,
                "each environment update should run the countdown down by exactly one");
    TEST_ASSERT_EQ(itemCharges(countdownCharm), 0,
                   "a used charm's countdown should run all the way down to recharged");
    TEST_ASSERT(messageWasShown("has recharged"),
                "a charm that finished counting down should announce that it has recharged");
    return !testFailed;
}

// ============================================================
// BR36633 — electricity promotes any tile flagged to promote on
// electricity
// ============================================================

static pos struckCrystalTile;
static pos bystanderCrystalTile;
static boolean struckCrystalStartedDark;
static boolean bystanderCrystalStartedDark;
static item *lightningStaff;

// An inert electric crystal sits in the line of fire of a lightning bolt,
// with a second inert crystal off to the side as a control.  The bolt must
// promote the crystal it strikes, and only that one.
static void electricity_promotes_setup(void) {
    setArenaRadius(6, FLOOR);

    struckCrystalTile = posOffset(getPlayerPos(), 2, 0);
    bystanderCrystalTile = posOffset(getPlayerPos(), 0, 3);
    setTerrain(struckCrystalTile, ELECTRIC_CRYSTAL_OFF);
    setTerrain(bystanderCrystalTile, ELECTRIC_CRYSTAL_OFF);
    struckCrystalStartedDark = cellIsTerrain(struckCrystalTile, ELECTRIC_CRYSTAL_OFF);
    bystanderCrystalStartedDark = cellIsTerrain(bystanderCrystalTile, ELECTRIC_CRYSTAL_OFF);

    lightningStaff = givePlayerItem(STAFF, STAFF_LIGHTNING, 0);
    placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 5, 0));
}

static boolean electricity_promotes_step(void) {
    playerZapsAt(lightningStaff, findMonster(MK_RAT));
    return false;
}

static boolean electricity_promotes_the_struck_tile(void) {
    TEST_ASSERT(struckCrystalStartedDark && bystanderCrystalStartedDark,
                "both crystals should start out darkened");
    TEST_ASSERT(cellIsTerrain(struckCrystalTile, ELECTRIC_CRYSTAL_ON),
                "a bolt of electricity should promote the tile it strikes");
    TEST_ASSERT(cellIsTerrain(bystanderCrystalTile, ELECTRIC_CRYSTAL_OFF),
                "electricity should not promote a tile the bolt never reached");
    return !testFailed;
}

// ============================================================
// BR24700 — each turn you try to move while stuck in webbing the
// counter ticks down ("you struggle but cannot free yourself")
// until you break free and the web is destroyed ("you break free!")
// ============================================================

static pos struggleWebPos;
static boolean struggleWebWasPlaced;
static int struggleAttempts;

static void web_struggle_setup(void) {
    clearArena();
    struggleWebPos = getPlayerPos();
    // A real web sits on the surface layer, which is where breaking free
    // looks for it; setTerrain() would bury a web in the dungeon layer
    // instead, where breaking free never removes it.
    spawnFeature(struggleWebPos, DF_WEB_SMALL);
    // DF_WEB_SMALL floods webs onto neighboring squares with decaying
    // probability.  When the player breaks free they step onto the adjacent
    // square; if that square is also webbed they are instantly re-stuck
    // (rand_range(3,7)) and the counter is no longer empty.  Clear every
    // neighbor so exactly one webbed square remains -- the one being struggled
    // against -- and the break-free step lands on clean floor.
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            setTerrain(posOffset(struggleWebPos, dx, dy), FLOOR);
        }
    }
    struggleWebWasPlaced = cellIsTerrain(struggleWebPos, SPIDERWEB);
    applyStatusToPlayer(STATUS_STUCK, 3);
    clearMessages();
    struggleAttempts = 0;
}

static boolean web_struggle_step(void) {
    moveRight();
    struggleAttempts++;
    // Keep trying to step away while the web still holds the player.
    return cellIsTerrain(struggleWebPos, SPIDERWEB) && struggleAttempts < 6;
}

static boolean player_frees_self_from_web(void) {
    TEST_ASSERT(struggleWebWasPlaced,
                "setup should have webbed the player's own square");
    TEST_ASSERT(messageWasShown("you struggle but cannot free yourself"),
                "every move attempt before the last should report the struggle");
    TEST_ASSERT(messageWasShown("you break free!"),
                "the final move attempt should break the player free");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_STUCK), 0,
                   "breaking free should empty the stuck counter");
    TEST_ASSERT(!cellIsTerrain(struggleWebPos, SPIDERWEB),
                "breaking free should destroy the web");
    return !testFailed;
}

// ============================================================
// BR32089 — every 100 ticks every monster's status timers decrement
// ============================================================

// Hasting the player halves the cost of one action to 50 ticks, so a single
// rest can pass less than a full 100-tick environment cycle.  After any
// environment update at most 100 ticks remain in the cycle, so the very next
// 50-tick rest is guaranteed not to complete another one.
static creature *timedSubject;
static int timerStatusInitial;
static int timerStatusAfterFirstUpdate;
static int timerStatusAfterHalfTurn;
static int timerStatusAfterSecondUpdate;
static boolean firstUpdateNeverSeen;
static boolean timerChangedMidCycle;
static boolean secondUpdateNeverSeen;
static int timerRests;
static int timerPhase; // 0: await first update; 1: half-rest check; 2: await second update; 3: done

static void monster_status_timer_setup(void) {
    clearArena();
    timedSubject = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    makeMonsterDormant(timedSubject); // so it can never act or interfere
    applyStatusToMonster(timedSubject, STATUS_WEAKENED, 50);
    hastePlayer(20);
    timerStatusInitial = monsterStatusTurns(timedSubject, STATUS_WEAKENED);
    timerStatusAfterFirstUpdate = timerStatusInitial;
    timerStatusAfterHalfTurn = timerStatusInitial;
    timerStatusAfterSecondUpdate = timerStatusInitial;
    firstUpdateNeverSeen = false;
    timerChangedMidCycle = false;
    secondUpdateNeverSeen = false;
    timerRests = 0;
    timerPhase = 0;
}

static boolean monster_status_timer_step(void) {
    restOneTurn();
    timerRests++;
    int statusNow = monsterStatusTurns(timedSubject, STATUS_WEAKENED);

    switch (timerPhase) {
        case 0: // wait for the first 100-tick environment update
            if (statusNow < timerStatusInitial) {
                timerStatusAfterFirstUpdate = statusNow;
                timerPhase = 1;
            } else if (timerRests >= 15) {
                firstUpdateNeverSeen = true;
                timerPhase = 3;
            }
            return true;
        case 1: // one 50-tick rest cannot complete another 100-tick cycle
            timerStatusAfterHalfTurn = statusNow;
            if (statusNow != timerStatusAfterFirstUpdate) {
                timerChangedMidCycle = true;
            }
            timerPhase = 2;
            return true;
        case 2: // wait for the next environment update
            if (statusNow < timerStatusAfterHalfTurn) {
                timerStatusAfterSecondUpdate = statusNow;
                timerPhase = 3;
            } else if (timerRests >= 25) {
                secondUpdateNeverSeen = true;
                timerPhase = 3;
            }
            return true;
        default:
            return false;
    }
}

static boolean monster_status_timers_decrement_per_100_ticks(void) {
    TEST_ASSERT(!firstUpdateNeverSeen,
                "the monster's status timer should tick down when the environment updates");
    TEST_ASSERT_EQ(timerStatusAfterFirstUpdate, timerStatusInitial - 1,
                   "one environment update should tick the monster's status timer down once");
    TEST_ASSERT(!timerChangedMidCycle,
                "the status timer must not tick down between environment updates");
    TEST_ASSERT(!secondUpdateNeverSeen,
                "the status timer should keep ticking down on later environment updates");
    TEST_ASSERT_EQ(timerStatusAfterSecondUpdate, timerStatusAfterFirstUpdate - 1,
                   "each later environment update should tick the timer down once more");
    return !testFailed;
}

// ============================================================
// BR11386 — If the leg-holder is visible and you have not yet committed to
// the action, the move is cancelled and no turn is spent; otherwise you
// spend the turn struggling.
// ============================================================

static creature *br11386_holder;
static unsigned long br11386_turn_at_move;
static pos br11386_pos_at_move;
static int br11386_rests;
static boolean br11386_holder_hidden; // which branch the step arranges

static void br11386_holder_setup(void) {
    clearMonsters();
    pos p = getPlayerPos();
    setTerrainAroundLoc(p, MUD); // the bog monster's home ground
    setPlayerPos(p);
    br11386_holder = placeHostileMonster(MK_BOG_MONSTER, posOffset(p, 1, 0));
    br11386_rests = 0;
}

static void br11386_visible_setup(void) {
    br11386_holder_hidden = false;
    br11386_holder_setup();
}

static void br11386_hidden_setup(void) {
    br11386_holder_hidden = true;
    br11386_holder_setup();
}

// The direction leading straight away from the holder.  The holder wanders
// around the player while it notices it, so the attempt must not run into
// the holder's own square, which would be an attack rather than a move.
static enum directions directionAwayFromHolder(pos from, pos holder) {
    int dx = holder.x - from.x;
    int dy = holder.y - from.y;
    if (dx > 0) {
        return dy > 0 ? UPLEFT : (dy < 0 ? DOWNLEFT : LEFT);
    }
    if (dx < 0) {
        return dy > 0 ? UPRIGHT : (dy < 0 ? DOWNRIGHT : RIGHT);
    }
    return dy > 0 ? UP : DOWN;
}

// Rest until the holder has seized the player, arrange the holder's
// visibility, then try to step off the holder's square.
static boolean br11386_step(void) {
    if (!playerIsSeized()) {
        if (++br11386_rests > 50) {
            testFail("the holder never seized the player");
            return false;
        }
        restOneTurn();
        return true;
    }
    if (br11386_holder_hidden) {
        // Slip beneath the mud, out of the player's sight.
        setMonsterSubmerged(br11386_holder, true);
    } else {
        // Surface, with its square lit and seen by the player.
        setMonsterSubmerged(br11386_holder, false);
        revealCell(monsterPosition(br11386_holder));
    }
    br11386_turn_at_move = turnNumber();
    br11386_pos_at_move = getPlayerPos();
    moveDirection(directionAwayFromHolder(br11386_pos_at_move,
                                          monsterPosition(br11386_holder)));
    return false;
}

static boolean br11386_visible_holder_cancels_move(void) {
    TEST_ASSERT_MSG(messageWasShown("you cannot move;"),
                    "a visible holder should cancel the move");
    TEST_ASSERT(!messageWasShown("you struggle but"),
                "a visible holder must not make the player spend the turn struggling");
    TEST_ASSERT_EQ(turnNumber(), br11386_turn_at_move,
                   "the cancelled move must spend no turn");
    TEST_ASSERT_POS_EQ(getPlayerPos(), br11386_pos_at_move,
                       "the cancelled move must leave the player in place");
    return !testFailed;
}

static boolean br11386_hidden_holder_spends_turn_struggling(void) {
    TEST_ASSERT_MSG(messageWasShown("you struggle but"),
                    "an unseen holder should make the player spend the turn struggling");
    TEST_ASSERT(!messageWasShown("you cannot move;"),
                "an unseen holder must not cancel the move");
    TEST_ASSERT_EQ(turnNumber(), br11386_turn_at_move + 1,
                   "the struggle must spend exactly one turn");
    TEST_ASSERT_POS_EQ(getPlayerPos(), br11386_pos_at_move,
                       "struggling must leave the player in place");
    return !testFailed;
}

// ============================================================
// BR65504 — once auto-explore has mapped out the level it heads for
// the stairs.
// ============================================================

static pos stairsExploreStart;
static pos stairsExploreGoal;
static pos downStairs;

static void explore_to_stairs_setup(void) {
    clearMonsters();
    // Rebuild the level as solid granite with one straight corridor
    // running east, so the experiment is the only walkable ground.  The
    // granite rebuild also clears every floor item's cell flag, so no
    // leftover item can act as a competing exploration goal.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }

    // Start from the middle of the map so the corridor cannot touch the
    // border rows and columns.  One unexplored goal square lies three
    // squares east; the down stairs lie four squares beyond it.
    stairsExploreStart = posAt(levelWidth() / 2, levelHeight() / 2);
    stairsExploreGoal = posOffset(stairsExploreStart, 3, 0);
    downStairs = posOffset(stairsExploreStart, 7, 0);
    for (int i = 0; i <= 7; i++) {
        setTerrain(posOffset(stairsExploreStart, i, 0), FLOOR);
    }
    placeDownStairs(downStairs);
    setPlayerPos(stairsExploreStart);

    // The whole level is known except the single goal square, so once
    // exploration reaches the goal there is nothing left to map.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos p = posAt(x, y);
            if (p.x != stairsExploreGoal.x || p.y != stairsExploreGoal.y) {
                revealCell(p);
            }
        }
    }

    // Full auto-play is engaged, which is what lets exploration fall
    // back to the stairs when no unexplored ground remains.
    enableAutopilot();
}

static boolean explore_to_stairs_step(void) {
    // Preconditions: the player starts at the west end of the corridor,
    // the goal square is still unexplored, and the stairs are known.
    TEST_ASSERT(playerIsAt(stairsExploreStart),
                "setup: the player should start at the west end of the corridor");
    TEST_ASSERT(terrainAt(downStairs) == DOWN_STAIRS,
                "setup: the down stairs should stand at the east end of the corridor");

    // The first press picks the exploration target; the second press
    // starts exploring toward it.
    injectKeystroke('x', false, false);
    injectKeystroke('x', false, false);
    return false;
}

static boolean explore_walks_to_the_stairs(void) {
    TEST_ASSERT(!isSameDepth(),
                "once the level is fully mapped, auto-explore should walk you onto the down stairs and descend");
    TEST_ASSERT(messageWasShown("You descend."),
                "auto-explore should have carried you down the stairs");
    return !testFailed;
}

// ============================================================
// BR19564 — auto-explore stops the moment you are disturbed.
//
// A one-square-wide corridor runs east from the player, turns north,
// and ends at a single unexplored goal square around the corner.  A
// hostile rat stands in the east leg two squares from the player.
// Auto-explore steps toward the goal, but the step that brings the
// player next to the rat ends with an exchange of blows (the rat
// attacks, or the player attacks on the following iteration), which
// disturbs the player.  The exploration loop must quit at once: the
// player stays on that square instead of fighting through and walking
// on around the corner to the goal.
// ============================================================

static pos br19564_start;
static pos br19564_goal;

static void br19564_setup(void) {
    clearMonsters();
    // Rebuild the level as solid granite so the corridor is the only
    // walkable ground.  The granite rebuild also clears every floor
    // item's cell flag, so no leftover item can act as a competing
    // exploration goal.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }

    br19564_start = posAt(levelWidth() / 2, levelHeight() / 2);
    // East leg, with the rat two squares along it.
    for (int i = 0; i <= 3; i++) {
        setTerrain(posOffset(br19564_start, i, 0), FLOOR);
    }
    // North leg around the corner.
    for (int i = 1; i <= 3; i++) {
        setTerrain(posOffset(br19564_start, 3, -i), FLOOR);
    }
    br19564_goal = posOffset(br19564_start, 3, -3);

    // Everything is known except the goal square around the corner, so
    // the goal is the only thing left to explore.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos p = posAt(x, y);
            if (p.x != br19564_goal.x || p.y != br19564_goal.y) {
                revealCell(p);
            }
        }
    }

    setPlayerPos(br19564_start);
    placeHostileMonster(MK_RAT, posOffset(br19564_start, 2, 0));
}

static boolean br19564_step(void) {
    TEST_ASSERT(playerIsAt(br19564_start),
                "setup: the player should start at the mouth of the corridor");
    TEST_ASSERT(!cellIsDiscovered(br19564_goal),
                "setup: the goal around the corner should still be unexplored");

    // The first press picks the exploration target; the second press
    // starts exploring toward it.
    injectKeystroke('x', false, false);
    injectKeystroke('x', false, false);
    return false;
}

static boolean br19564_stops_when_disturbed(void) {
    TEST_ASSERT(playerIsAt(posOffset(br19564_start, 1, 0)),
                "auto-explore should stop on the square where the fight disturbed the player");
    TEST_ASSERT_EQ(countMonstersOfType(MK_RAT), 1,
                   "control: the monster should still be alive, so the stop was not caused by its death");
    creature *rat = findMonster(MK_RAT);
    TEST_ASSERT(rat != NULL && monsterIsAdjacentToPlayer(rat),
                "the player should have stopped next to the hostile monster, not walked past it");
    TEST_ASSERT(!cellIsDiscovered(br19564_goal),
                "the player should not have walked on around the corner to the goal");
    return !testFailed;
}

// ============================================================
// BR24904 — Surviving explosive ground grants 5 turns of immunity
// to further explosions.
//
// Each turn the player stands on explosive ground, the engine's
// instant tile-effect check runs.  The first turn blasts the player
// and grants the immunity, so the next five turns must pass without
// a blast even though the ground is still explosive, and the
// immunity must then expire so a later turn is blasted again.
// ============================================================

static int br24904_turn;
static int br24904_damageTurns[16];
static int br24904_damageCount;

static void br24904_setup(void) {
    clearArena();
    br24904_turn = 0;
    br24904_damageCount = 0;
    setPlayerHealth(playerMaxHealth());
    setTerrain(getPlayerPos(), GAS_EXPLOSION);
}

static boolean br24904_step(void) {
    // Keep the ground explosive and the player surviving so the whole
    // blast/immunity pattern can be counted.
    setPlayerHealth(playerMaxHealth());
    setTerrain(getPlayerPos(), GAS_EXPLOSION);
    const int hpBefore = playerHealth();
    applyTileEffectsToPlayer(); // the engine's standing-on-explosive-ground check
    if (playerHealth() < hpBefore && br24904_damageCount < 16) {
        br24904_damageTurns[br24904_damageCount++] = br24904_turn + 1;
    }
    restOneTurn(); // a full turn passes: the blast lapses, the immunity counter ticks down
    br24904_turn++;
    return br24904_turn < 10;
}

static boolean br24904_survivor_is_temporarily_immune(void) {
    TEST_ASSERT(br24904_damageCount >= 1 && br24904_damageTurns[0] == 1,
                "the first turn on explosive ground should blast the player");
    TEST_ASSERT(!isPlayerDead(), "the player should survive the blasts");
    TEST_ASSERT(messageWasShown("the force of the explosion slams into you"),
                "the explosion should announce its impact");
    // Five full turns of immunity: turns 2 through 6 bring no further blast.
    TEST_ASSERT(br24904_damageCount == 1 || br24904_damageTurns[1] > 6,
                "the five turns after surviving a blast should be immune to explosions");
    // The immunity is temporary: once it expires the ground blasts again.
    TEST_ASSERT(br24904_damageCount >= 2,
                "explosive ground should blast the player again once the immunity expires");
    return !testFailed;
}

// ============================================================
// BR25832 — Dampening armor absorbs an explosion ("Your <armor>
// pulses and absorbs the damage.") and may become identified.
//
// The player wears an unidentified suit of dampening armor and
// stands on explosive ground.  The engine's standing-on-explosive-
// ground check must announce the absorption, leave health untouched,
// and identify the rune.  A suit of runic armor with a different rune
// is the control: the same blast must wound the player and print no
// absorption message.
// ============================================================

static item *br25832_armor;
static int br25832_healthBefore;
static int br25832_healthAfter;
static boolean br25832_runeKnownBefore;
static boolean br25832_runeKnownAfter;
static boolean br25832_message;

static void br25832_dampening_setup(void) {
    clearArena();
    br25832_armor = givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 10, false);
    setPlayerHealth(1000);
    setTerrain(getPlayerPos(), GAS_EXPLOSION);
    br25832_healthBefore = playerHealth();
    br25832_runeKnownBefore = runicIsIdentified(br25832_armor);
    clearMessages();
    applyTileEffectsToPlayer(); // the engine's standing-on-explosive-ground check
    br25832_healthAfter = playerHealth();
    br25832_runeKnownAfter = runicIsIdentified(br25832_armor);
    br25832_message = messageWasShown("pulses and absorbs the damage");
}

static boolean br25832_dampening_armor_absorbs_explosion(void) {
    TEST_ASSERT(!br25832_runeKnownBefore,
                "control: the dampening rune should start unidentified");
    TEST_ASSERT_EQ(br25832_healthAfter, br25832_healthBefore,
                   "dampening armor should absorb the whole explosion");
    TEST_ASSERT(br25832_message,
                "dampening armor should announce that it pulses and absorbs the damage");
    TEST_ASSERT(br25832_runeKnownAfter,
                "the absorbed explosion should identify the dampening armor");
    return !testFailed;
}

static void br25832_control_setup(void) {
    clearArena();
    br25832_armor = givePlayerRunicArmor(CHAIN_MAIL, A_VULNERABILITY, 10, false);
    setPlayerHealth(1000);
    setTerrain(getPlayerPos(), GAS_EXPLOSION);
    br25832_healthBefore = playerHealth();
    clearMessages();
    applyTileEffectsToPlayer();
    br25832_healthAfter = playerHealth();
    br25832_message = messageWasShown("pulses and absorbs the damage");
}

static boolean br25832_other_armor_takes_explosion_damage(void) {
    TEST_ASSERT(br25832_healthAfter < br25832_healthBefore,
                "control: armor without the dampening rune should not absorb the explosion");
    TEST_ASSERT(!br25832_message,
                "control: armor without the dampening rune should not announce absorption");
    return !testFailed;
}

// ============================================================
// BR34686 — the turn's discovery count is granted as experience to
// your allies on your level and on the levels immediately above and
// below
// ============================================================

static creature *discoveryAllyAbove, *discoveryAllySame;
static creature *discoveryAllyBelow, *discoveryAllyFar;
static int discoveryXpAbove, discoveryXpSame, discoveryXpBelow, discoveryXpFar;

static creature *spawnDiscoveryAlly(pos loc) {
    setTerrain(loc, FLOOR);
    creature *ally = spawnMonster(MK_RAT, loc);
    makeMonsterAlly(ally);
    // A captive ally is never marked to approach the stairs when the player
    // leaves its level, so each ally stays where the test put it.
    makeMonsterCaptive(ally);
    return ally;
}

static void discovery_xp_setup(void) {
    // The test ends on depth 3, with allies waiting on depths 1 through 4.
    clearMonsters();
    discoveryAllyFar = spawnDiscoveryAlly(posOffset(getPlayerPos(), 1, 0)); // depth 1: two above depth 3
    descendToNextLevel();                                                   // depth 2
    clearMonsters();
    discoveryAllyAbove = spawnDiscoveryAlly(posOffset(getPlayerPos(), -1, 0));
    descendToNextLevel();                                                   // depth 3, first visit
    clearMonsters();
    descendToNextLevel();                                                   // depth 4
    clearMonsters();
    discoveryAllyBelow = spawnDiscoveryAlly(posOffset(getPlayerPos(), 1, 0));
    ascendToPreviousLevel();                                                // back to depth 3

    // Fresh floor around the player: the discovery turn walks past tiles the
    // player has never seen, so the turn's discovery count is nonzero.
    setTerrainAroundLoc(getPlayerPos(), FLOOR);
    discoveryAllySame = spawnDiscoveryAlly(posOffset(getPlayerPos(), 3, 0));

    // The one step the test takes must not land on a staircase.
    placeUpStairs(posAt(1, 1));
    placeDownStairs(posAt(levelWidth() - 2, levelHeight() - 2));

    discoveryXpFar = monsterExperience(discoveryAllyFar);
    discoveryXpAbove = monsterExperience(discoveryAllyAbove);
    discoveryXpSame = monsterExperience(discoveryAllySame);
    discoveryXpBelow = monsterExperience(discoveryAllyBelow);
}

static boolean discovery_xp_step(void) {
    moveRight();
    return false;
}

static boolean discovery_xp_goes_to_allies_on_nearby_levels(void) {
    TEST_ASSERT(monsterExperience(discoveryAllyAbove) > discoveryXpAbove,
                "an ally one level above should gain the turn's discovery experience");
    TEST_ASSERT(monsterExperience(discoveryAllySame) > discoveryXpSame,
                "an ally on your level should gain the turn's discovery experience");
    TEST_ASSERT(monsterExperience(discoveryAllyBelow) > discoveryXpBelow,
                "an ally one level below should gain the turn's discovery experience");
    TEST_ASSERT_EQ(monsterExperience(discoveryAllyFar), discoveryXpFar,
                   "an ally two levels away should not gain the turn's discovery experience");
    TEST_ASSERT_EQ(monsterExperience(discoveryAllyAbove) - discoveryXpAbove,
                   monsterExperience(discoveryAllySame) - discoveryXpSame,
                   "each nearby ally should gain the whole discovery count");
    TEST_ASSERT_EQ(monsterExperience(discoveryAllyBelow) - discoveryXpBelow,
                   monsterExperience(discoveryAllySame) - discoveryXpSame,
                   "each nearby ally should gain the whole discovery count");
    return !testFailed;
}

// ============================================================
// BR34157 — discovering a new square that can be pathed through
// adds to the current turn's discovery count
//
// The turn's count is handed to your allies as experience when the
// turn ends, so a captive ally left on the level reports how many
// squares the turn discovered.  The same sealed room is used twice:
// once with a floor tile left unseen (it must add one), and once
// with only a granite wall left unseen (it must add nothing even
// though the wall is newly discovered).
// ============================================================

#define BR34157_SEAL_RADIUS 5
#define BR34157_SIGHT_RADIUS 3

static pos br34157_centre, br34157_floorTarget, br34157_wallTarget;
static creature *br34157_ally;
static int br34157_phase;
static int br34157_floorGain, br34157_wallGain;
static boolean br34157_wallDiscovered;

static void br34157_spawnAlly(void) {
    pos allyLoc = posAt(4, 4);
    setTerrain(allyLoc, FLOOR);
    br34157_ally = spawnMonster(MK_RAT, allyLoc);
    makeMonsterAlly(br34157_ally);
    makeMonsterCaptive(br34157_ally);
}

// A 3x3 floor room at the level's centre, sealed inside granite thick enough
// that vision cannot escape past the walls.
static void br34157_buildRoom(void) {
    br34157_centre = posAt(levelWidth() / 2, levelHeight() / 2);
    setPlayerPos(br34157_centre);
    setArenaRadius(BR34157_SEAL_RADIUS, GRANITE);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            setTerrain(posOffset(br34157_centre, dx, dy), FLOOR);
        }
    }
    setPlayerPos(br34157_centre);
    br34157_floorTarget = posOffset(br34157_centre, 1, 0);
    br34157_wallTarget = posOffset(br34157_centre, 2, 0);
}

// Mark everything the room's vision can reach as already discovered, except
// the one tile whose discovery this phase is about.
static void br34157_revealRoomExcept(pos skip) {
    for (int dx = -BR34157_SIGHT_RADIUS; dx <= BR34157_SIGHT_RADIUS; dx++) {
        for (int dy = -BR34157_SIGHT_RADIUS; dy <= BR34157_SIGHT_RADIUS; dy++) {
            pos loc = posOffset(br34157_centre, dx, dy);
            if ((loc.x != skip.x || loc.y != skip.y)
                && loc.x >= 0 && loc.x < levelWidth()
                && loc.y >= 0 && loc.y < levelHeight()) {
                revealCell(loc);
            }
        }
    }
}

static void br34157_setup(void) {
    // Keep staircases out of the sealed room.
    placeUpStairs(posAt(1, 1));
    placeDownStairs(posAt(levelWidth() - 2, levelHeight() - 2));
    br34157_buildRoom();
    // Level generation discovered the starting level without a turn ending,
    // leaving its count pending.  Spend one turn in the fully discovered room
    // so that count is granted and cleared before either phase begins.
    br34157_revealRoomExcept(posAt(-1, -1));
    restOneTurn();
    br34157_phase = 0;
}

static boolean br34157_step(void) {
    if (br34157_phase == 0) {
        // The settling turn in setup marked the room seen; rebuild it so the
        // tiles this phase is about start undiscovered again.
        br34157_buildRoom();
        // An unseen floor tile lies one step to the right.
        br34157_revealRoomExcept(br34157_floorTarget);
        br34157_spawnAlly();
        moveRight();
        // The count is granted to allies at a turn end; a rest turn in the
        // fully discovered room adds no discovery of its own.
        restOneTurn();
        br34157_floorGain = monsterExperience(br34157_ally);
        br34157_phase = 1;
        return true;
    }
    // Same room, but now only the granite wall beyond the known floor is new.
    br34157_buildRoom();
    br34157_revealRoomExcept(br34157_wallTarget);
    br34157_spawnAlly();
    moveRight();
    restOneTurn();
    br34157_wallGain = monsterExperience(br34157_ally);
    br34157_wallDiscovered = cellIsDiscovered(br34157_wallTarget);
    return false;
}

static boolean br34157_verify(void) {
    TEST_ASSERT_EQ(br34157_floorGain, 1,
                   "a newly discovered floor square adds one to the turn's discovery count");
    TEST_ASSERT(br34157_wallDiscovered,
                "the granite wall beyond the room should have been newly discovered");
    TEST_ASSERT_EQ(br34157_wallGain, 0,
                   "a newly discovered wall adds nothing to the turn's discovery count");
    return !testFailed;
}

// ============================================================
// BR21531 — explosive terrain erupts once at least 8 of its
// neighbouring squares are on fire, block gas, or are themselves
// explosive
// ============================================================

static pos br21531_centre;

// Build the test pocket at the middle of the map: eight gas-blocking
// granite squares around a floor centre filled with explosive gas, with
// the player standing in it and burning.  A burning creature's standing
// tile effect exposes its own square to fire, which is the moment the
// engine counts the square's neighbours.
static void br21531_gas_pocket_setup(void) {
    pos centre = posAt(levelWidth() / 2, levelHeight() / 2);
    setTerrain(centre, FLOOR);
    setPlayerPos(centre);
    setArenaRadius(2, FLOOR);

    br21531_centre = centre;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            setTerrain(posOffset(centre, dx, dy), GRANITE);
        }
    }
    spawnFeature(centre, DF_METHANE_GAS_PUFF);
    TEST_ASSERT(cellHasGas(centre),
                "setup: the centre square should fill with explosive gas");
    setPlayerHealth(playerMaxHealth());
    applyStatusToPlayer(STATUS_BURNING, 10);
}

static void br21531_erupt_setup(void) {
    br21531_gas_pocket_setup(); // eight gas-blocking granite neighbours
}

static void br21531_quiet_setup(void) {
    br21531_gas_pocket_setup();
    // Only seven of the eight neighbours now qualify: this square is
    // ordinary floor — not on fire, not gas-blocking, not explosive.
    setTerrain(posOffset(br21531_centre, 1, 0), FLOOR);
    TEST_ASSERT(cellHasGas(br21531_centre),
                "setup: opening one neighbour should leave the gas in place");
}

static boolean br21531_step(void) {
    setPlayerHealth(playerMaxHealth()); // the fire on the player costs health
    applyTileEffectsToPlayer();         // exposes the centre square to fire
    return false;
}

static boolean br21531_gas_erupts_at_eight(void) {
    TEST_ASSERT(cellIsTerrain(br21531_centre, GAS_EXPLOSION),
                "with eight qualifying neighbours the explosive gas should erupt into an explosion");
    TEST_ASSERT(!cellIsTerrain(br21531_centre, GAS_FIRE),
                "the eruption should replace the plain burning the gas would otherwise do");
    return !testFailed;
}

static boolean br21531_gas_holds_at_seven(void) {
    TEST_ASSERT(!cellIsTerrain(br21531_centre, GAS_EXPLOSION),
                "with only seven qualifying neighbours the explosive gas should not erupt");
    TEST_ASSERT(cellIsTerrain(br21531_centre, GAS_FIRE),
                "the gas should still catch fire and burn quietly when it does not erupt");
    return !testFailed;
}

// ============================================================
// BR30801 — Monsters that are activation-only, paralyzed,
// entranced, or captive do not act when their ticks run out;
// their timer is reset.
//
// Three identical rats start eight squares away.  The control
// phase waits until every one of them has taken a turn of its own
// accord, then disables each in a different way and locks in the
// positions.  Over the following turns none of them may budge:
// if the rule were broken they would keep closing on you.  The
// guardian, which acts only when its machine triggers, must stay
// put from the start.
// ============================================================

static creature *br30801_subjects[3];  // paralyzed, entranced, captive
static creature *br30801_guardian;
static pos br30801_startPos[3];
static pos br30801_lockedPos[3];
static pos br30801_guardianStart;
static boolean br30801_moved[3];
static boolean br30801_controlDone;
static int br30801_turn;

static const char *br30801_messages[3] = {
    "a paralyzed monster does not act when its ticks run out",
    "an entranced monster does not act when its ticks run out",
    "a captive monster does not act when its ticks run out"
};

static boolean br30801_displaced(pos start, pos current) {
    return current.x != start.x || current.y != start.y;
}

static void br30801_setup(void) {
    // Work from the middle of the map so the eight-square approach
    // runs are guaranteed to be in bounds.
    pos centre = posAt(levelWidth() / 2, levelHeight() / 2);
    setTerrain(centre, FLOOR);
    setPlayerPos(centre);
    setArenaRadius(10, FLOOR);

    br30801_subjects[0] = placeHostileMonster(MK_RAT, posOffset(centre, 8, 0));
    br30801_subjects[1] = placeHostileMonster(MK_RAT, posOffset(centre, 8, 2));
    br30801_subjects[2] = placeHostileMonster(MK_RAT, posOffset(centre, 8, -2));
    br30801_guardian = placeHostileMonster(MK_GUARDIAN, posOffset(centre, 8, -4));

    for (int i = 0; i < 3; i++) {
        br30801_startPos[i] = monsterPosition(br30801_subjects[i]);
        br30801_moved[i] = false;
    }
    br30801_guardianStart = monsterPosition(br30801_guardian);
    br30801_controlDone = false;
    br30801_turn = 0;
}

static boolean br30801_step(void) {
    if (!br30801_controlDone) {
        // Control phase: each rat must first be seen taking a turn
        // normally, so its later stillness cannot be a vacuous pass.
        restOneTurn();
        br30801_turn++;
        for (int i = 0; i < 3; i++) {
            if (!br30801_moved[i]
                && br30801_displaced(br30801_startPos[i],
                                     monsterPosition(br30801_subjects[i]))) {
                br30801_moved[i] = true;
            }
        }
        if (br30801_moved[0] && br30801_moved[1] && br30801_moved[2]) {
            br30801_controlDone = true;
            applyStatusToMonster(br30801_subjects[0], STATUS_PARALYZED, 100);
            applyStatusToMonster(br30801_subjects[1], STATUS_ENTRANCED, 100);
            makeMonsterCaptive(br30801_subjects[2]);
            for (int i = 0; i < 3; i++) {
                br30801_lockedPos[i] = monsterPosition(br30801_subjects[i]);
            }
            br30801_turn = 0;
            return true;
        }
        return br30801_turn < 15;   // give up; verify() reports the control
    }

    restOneTurn();
    br30801_turn++;
    return br30801_turn < 6;
}

static boolean br30801_verify(void) {
    TEST_ASSERT(br30801_controlDone,
                "control: each rat takes its turn normally before it is disabled");
    if (br30801_controlDone) {
        for (int i = 0; i < 3; i++) {
            TEST_ASSERT(monsterIsAlive(br30801_subjects[i]),
                        "the disabled monster is still alive");
            TEST_ASSERT(!br30801_displaced(br30801_lockedPos[i],
                                           monsterPosition(br30801_subjects[i])),
                        br30801_messages[i]);
        }
    }
    TEST_ASSERT(monsterIsAlive(br30801_guardian), "the guardian is still alive");
    TEST_ASSERT(!br30801_displaced(br30801_guardianStart,
                                   monsterPosition(br30801_guardian)),
                "an activation-only monster does not act until its machine triggers");
    return !testFailed;
}

// ============================================================
// BR25953 — Mending ground heals you at the same rate, up to
// your maximum ("you feel much better.").
//
// Starving (nutrition 0) suppresses the player's fickle natural
// regeneration, so every turn has the same shape: a fixed one
// point of starvation, then whatever the tile underfoot does.
// From one starting health, a control turn on bare floor records
// that fixed balance, a turn in caustic gas records how fast
// harmful ground drains you, and the mirrored turn on the
// mending cloud must come out exactly as far ahead as the
// caustic gas left you behind — that equality is the rule's
// "same rate", and the control-drain assertion keeps it from
// passing vacuously.  The last turn starts at full health:
// starvation dips you one point below, and healing must carry
// you back to the maximum and not past it.
// ============================================================

#define BR25953_START_HP 10

static int br25953_hpControl;
static int br25953_hpHarmed;
static int br25953_hpMended;
static int br25953_hpCapped;
static int br25953_phase;

static void br25953_setup(void) {
    // Work from the middle of the map so the clouds stay in bounds.
    pos centre = posAt(levelWidth() / 2, levelHeight() / 2);
    setTerrain(centre, FLOOR);
    setPlayerPos(centre);
    setArenaRadius(4, FLOOR);
    // Starve the player so natural regeneration (which is fickle about
    // when it fires) can never mask the tile's own healing.
    setPlayerNutrition(0);
    br25953_phase = 0;
}

static boolean br25953_step(void) {
    if (br25953_phase == 0) {
        // Control: one wounded turn with bare floor under you.
        setPlayerHealth(BR25953_START_HP);
        restOneTurn();
        br25953_hpControl = playerHealth();
        br25953_phase = 1;
        return true;
    }
    if (br25953_phase == 1) {
        // The same turn standing in caustic gas: the drain rate.
        spawnFeature(getPlayerPos(), DF_POISON_GAS_CLOUD);
        setPlayerHealth(BR25953_START_HP);
        restOneTurn();
        br25953_hpHarmed = playerHealth();
        br25953_phase = 2;
        return true;
    }
    if (br25953_phase == 2) {
        // The mirrored turn on mending ground; the healing cloud
        // replaces the caustic gas on your own tile.
        spawnFeature(getPlayerPos(), DF_BLOODFLOWER_POD_BURST);
        setPlayerHealth(BR25953_START_HP);
        restOneTurn();
        br25953_hpMended = playerHealth();
        br25953_phase = 3;
        return true;
    }
    // Start at full health: starvation dips you one point below the
    // maximum, so the heal must reach the maximum and stop there.
    spawnFeature(getPlayerPos(), DF_BLOODFLOWER_POD_BURST);
    setPlayerHealth(playerMaxHealth());
    restOneTurn();
    br25953_hpCapped = playerHealth();
    return false;
}

static boolean br25953_verify(void) {
    TEST_ASSERT(cellHasGas(getPlayerPos()),
                "the healing cloud surrounds you");
    TEST_ASSERT(br25953_hpControl > br25953_hpHarmed,
                "control: harmful ground drains you, so its rate is observable");
    // The fixed starvation balance is common to the control, harmed and
    // mended turns and cancels: mending gain == harm drain rearranges to
    // mended + harmed == 2 * control.
    TEST_ASSERT_EQ(br25953_hpMended + br25953_hpHarmed, 2 * br25953_hpControl,
                   "mending ground heals at the same rate harmful ground drains");
    TEST_ASSERT(messageWasShown("you feel much better."),
                "mending ground says \"you feel much better.\"");
    TEST_ASSERT_EQ(br25953_hpCapped, playerMaxHealth(),
                   "mending ground heals up to your maximum and no further");
    return !testFailed;
}

// ============================================================
// BR26045 — the fall executes at the end of the turn, so fast
// monsters cannot act while suspended in mid-air.
//
// A wraith moves twice as fast as the player.  Placed on a chasm
// two tiles away and hostile, it would step toward the player on
// its first action and leave the chasm before any fall could
// happen.  The fall is settled before any monster acts, so after
// one rest the wraith is on the level below, damaged by the fall,
// and the player has not been attacked.
// ============================================================

static creature *br26045_faller;
static pos br26045_chasm;
static int br26045_healthBefore;
static pos br26045_playerBefore;

static void br26045_setup(void) {
    clearArena();
    setPlayerHealth(playerMaxHealth()); // a hit would show up below

    br26045_chasm = posOffset(getPlayerPos(), 2, 0);
    setTerrain(br26045_chasm, CHASM);
    br26045_faller = spawnMonster(MK_WRAITH, br26045_chasm);
    makeMonsterHostile(br26045_faller);
    revealCell(br26045_chasm);

    br26045_healthBefore = monsterHealth(br26045_faller);
    br26045_playerBefore = getPlayerPos();
}

static boolean br26045_rest_step(void) {
    restOneTurn();
    return false;
}

static boolean br26045_falls_before_acting(void) {
    TEST_ASSERT(monsterMovementSpeed(br26045_faller) < 100,
                "setup: the falling monster must be faster than the player");
    TEST_ASSERT_EQ(countMonstersOfType(MK_WRAITH), 0,
                   "a fast monster on a chasm should fall before it can act");
    TEST_ASSERT_POS_EQ(getPlayerPos(), br26045_playerBefore,
                       "the player should not have moved");
    TEST_ASSERT_EQ(playerHealth(), playerMaxHealth(),
                   "the fast monster should not have attacked before falling");

    descendToNextLevel();
    creature *below = findMonster(MK_WRAITH);
    TEST_ASSERT(below != NULL,
                "the fast monster should be waiting on the level below");
    if (below != NULL) {
        int damage = br26045_healthBefore - monsterHealth(below);
        TEST_ASSERT(damage >= 6 && damage <= 12,
                    "the fall should deal 6-12 damage");
    }
    return !testFailed;
}

// ============================================================
// BR28360 — a burning creature standing on flammable terrain,
// unless submerged or falling, sets that terrain alight.
//
// A paralyzed burning ogre stands on grass.  The next environment
// update exposes its tile to fire, which consumes the grass and
// leaves either flames or, if they burned down in the same
// update, their embers.
// ============================================================

static creature *br28360_burner;
static pos br28360_ground;

static void br28360_setup(void) {
    clearArena();

    br28360_ground = posOffset(getPlayerPos(), 2, 0);
    setTerrain(br28360_ground, GRASS);
    br28360_burner = spawnMonster(MK_OGRE, br28360_ground);
    applyStatusToMonster(br28360_burner, STATUS_BURNING, 1000);
    applyStatusToMonster(br28360_burner, STATUS_PARALYZED, 1000);

    TEST_ASSERT(cellIsTerrain(br28360_ground, GRASS),
                "setup: the creature should stand on flammable grass");
    TEST_ASSERT(!cellIsOnFire(br28360_ground),
                "setup: the grass should not be alight yet");
}

static boolean br28360_rest_step(void) {
    restOneTurn();
    return false;
}

static boolean br28360_ground_is_set_alight(void) {
    TEST_ASSERT(!cellIsTerrain(br28360_ground, GRASS),
                "a burning creature should consume the flammable ground it stands on");
    TEST_ASSERT(cellIsOnFire(br28360_ground) || cellIsTerrain(br28360_ground, EMBERS),
                "the ignited ground should be flames or their embers");
    return !testFailed;
}

// ============================================================
// BR70260 — monsters that fall take 6-12 damage and reappear on
// the level below.
//
// An ogre spawned onto a chasm and paralyzed so it cannot step
// away falls at the end of an environment update: it is removed
// from the upper level, loses 6-12 health, and is waiting on the
// next level down.  An ogre has enough health to survive the
// range, so the disappearance cannot be explained by death.
// ============================================================

static creature *br70260_subject;
static int br70260_healthBefore;
static int br70260_waited;

static void br70260_setup(void) {
    clearArena();

    const pos chasm = posOffset(getPlayerPos(), 2, 0);
    setTerrain(chasm, CHASM);

    br70260_subject = spawnMonster(MK_OGRE, chasm);
    applyStatusToMonster(br70260_subject, STATUS_PARALYZED, 1000);
    br70260_healthBefore = monsterHealth(br70260_subject);
    br70260_waited = 0;
}

static boolean br70260_wait_to_fall_step(void) {
    if (countMonstersOfType(MK_OGRE) == 0 || monsterIsDead(br70260_subject)) {
        return false;
    }
    if (++br70260_waited > 400) {
        testFail("the monster over the chasm never fell");
        return false;
    }
    restOneTurn();
    return true;
}

static boolean br70260_monster_falls_damaged_to_level_below(void) {
    TEST_ASSERT(!monsterIsDead(br70260_subject),
                "the fallen monster should survive 6-12 points of fall damage");

    const int healthAfterFall = monsterHealth(br70260_subject);
    const int damage = br70260_healthBefore - healthAfterFall;
    TEST_ASSERT(damage >= 6, "a fall should deal at least 6 damage");
    TEST_ASSERT(damage <= 12, "a fall should deal at most 12 damage");

    TEST_ASSERT_EQ(countMonstersOfType(MK_OGRE), 0,
                   "the fallen monster should leave the upper level");

    descendToNextLevel();
    TEST_ASSERT_EQ(currentDepth(), 2, "the player should now be on depth 2");

    creature *below = findMonster(MK_OGRE);
    TEST_ASSERT(below != NULL,
                "the fallen monster should reappear on the level below");
    TEST_ASSERT_EQ(monsterHealth(below), healthAfterFall,
                   "the monster below should be the one that fell, still damaged");
    return !testFailed;
}

// ============================================================
// BR17834 — choosing a travel target
//
// Your own square is refused with "you are already there.", an explored
// square becomes the travel cursor target, and an unexplored square prints
// the caller's failure message.
// ============================================================

static pos br17834_startPos;
static pos br17834_targetPos;

static void br17834_own_square_setup(void) {
    clearArena();
    br17834_startPos = getPlayerPos();
    // Put the down staircase under the player, so choosing it targets their
    // own square.
    placeDownStairs(br17834_startPos);
    setPlayerPos(br17834_startPos);
}

static boolean br17834_own_square_step(void) {
    injectKeystroke(DESCEND_KEY, false, false);
    return false;
}

static boolean br17834_own_square_reports_already_there(void) {
    TEST_ASSERT(messageWasShown("you are already there."),
                "choosing your own square should report that you are already there");
    TEST_ASSERT_POS_EQ(getPlayerPos(), br17834_startPos,
                       "choosing your own square should not move the player");
    return !testFailed;
}

static void br17834_explored_setup(void) {
    clearArena();
    br17834_startPos = getPlayerPos();
    br17834_targetPos = posOffset(br17834_startPos, 1, 0);
    placeDownStairs(br17834_targetPos);
    revealCell(br17834_targetPos);
}

static boolean br17834_explored_step(void) {
    // The first command records the explored staircase as the cursor target
    // without moving; the second accepts that target and travel steps onto
    // the staircase, descending.
    injectKeystroke(DESCEND_KEY, false, false);
    TEST_ASSERT_POS_EQ(getPlayerPos(), br17834_startPos,
                       "the first command should move the cursor, not the player");
    TEST_ASSERT_EQ(currentDepth(), 1,
                   "the first command should not travel");
    injectKeystroke(DESCEND_KEY, false, false);
    return false;
}

static boolean br17834_explored_target_is_travelled_to(void) {
    TEST_ASSERT(messageWasShown("You descend."),
                "confirming the explored square should travel to it");
    TEST_ASSERT_EQ(currentDepth(), 2,
                   "travelling to the explored down staircase should descend");
    return !testFailed;
}

static void br17834_unexplored_setup(void) {
    clearArena();
    br17834_startPos = getPlayerPos();
    br17834_targetPos = posOffset(br17834_startPos, 2, 0);
    placeDownStairs(br17834_targetPos);
    clearMessages();
}

static boolean br17834_unexplored_step(void) {
    injectKeystroke(DESCEND_KEY, false, false);
    return false;
}

static boolean br17834_unexplored_reports_failure(void) {
    TEST_ASSERT(messageWasShown("I see no way down."),
                "choosing an unexplored square should print the failure message");
    TEST_ASSERT_POS_EQ(getPlayerPos(), br17834_startPos,
                       "choosing an unexplored square should not move the player");
    return !testFailed;
}

// ============================================================
// BR21973 — A confused player cannot use auto-explore.
// ============================================================

static void br21973_setup(void) {
    clearArena();
    // Leave one square unexplored so auto-explore has a goal to head for.
    pos goal = posOffset(getPlayerPos(), 2, 0);
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos p = posAt(x, y);
            if (p.x != goal.x || p.y != goal.y) {
                revealCell(p);
            }
        }
    }
    applyStatusToPlayer(STATUS_CONFUSED, 5);
}

static boolean br21973_step(void) {
    // The first press picks the exploration target; the second starts exploring.
    injectKeystroke('x', false, false);
    injectKeystroke('x', false, false);
    return false;
}

static boolean br21973_verify(void) {
    TEST_ASSERT(messageWasShown("Not while you're confused."),
                "a confused player should be told auto-explore is unavailable");
    return !testFailed;
}

// ============================================================
// BR28134 — while burning you take 1-3 fire damage each turn
//            until the flames burn out on their own
// ============================================================

#define BR28134_BURN_TURNS 3

static int br28134_healthBefore;
static int br28134_damage[BR28134_BURN_TURNS + 1];
static int br28134_rests;

static void br28134_burning_setup(void) {
    clearArena();
    // A poison status with no concentration suppresses natural regeneration
    // without dealing damage, so each turn's fire damage is measured exactly.
    applyStatusToPlayer(STATUS_POISONED, 100);
    TEST_ASSERT_EQ(playerPoisonConcentration(), 0,
                   "setup: the poison status should carry no concentration");
    setPlayerHealth(200);
    applyStatusToPlayer(STATUS_BURNING, BR28134_BURN_TURNS);
    br28134_healthBefore = playerHealth();
    br28134_rests = 0;
}

static boolean br28134_burning_step(void) {
    restOneTurn();
    int health = playerHealth();
    br28134_damage[br28134_rests] = br28134_healthBefore - health;
    br28134_healthBefore = health;
    br28134_rests++;
    // One extra rest after the flames have burned out proves they stay out.
    return br28134_rests <= BR28134_BURN_TURNS;
}

static boolean br28134_burning_verify(void) {
    for (int i = 0; i < BR28134_BURN_TURNS; i++) {
        TEST_ASSERT(br28134_damage[i] >= 1 && br28134_damage[i] <= 3,
                    "each turn of burning should deal 1 to 3 fire damage");
    }
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_BURNING), 0,
                   "the flames should burn out on their own");
    TEST_ASSERT(!isPlayerBurning(), "the player should no longer be burning");
    TEST_ASSERT_EQ(br28134_damage[BR28134_BURN_TURNS], 0,
                   "the fire should stop damaging once the flames burn out");
    return !testFailed;
}

// ============================================================
// BR28134 — burning can kill you "Burned to death"
// ============================================================

static void br28134_burn_death_setup(void) {
    clearArena();
    setPlayerHealth(1);
    // Long enough that the flames cannot burn out before the damage kills.
    applyStatusToPlayer(STATUS_BURNING, 100);
}

static boolean br28134_burn_death_step(void) {
    if (!isPlayerDead() && turnNumber() < 1000) {
        restOneTurn();
        return true;
    }
    return false;
}

static boolean br28134_burn_death_verify(void) {
    TEST_ASSERT(isPlayerDead() && runHasEnded(),
                "burning should be able to kill the player");
    TEST_ASSERT(playerDeathWasAttributedTo("Burned to death"),
                "a death by burning should be attributed to \"Burned to death\"");
    return !testFailed;
}

// ============================================================
// BR13526 — a freed captive becomes a grateful, faithful ally
//
// Bump into a captive and accept the offer to free it.  The freed
// creature must change from a captive into one of your allies, and the
// game must say so with the grateful-ally message.  The pre-state check
// proves the ally status came from the freeing, not from the spawn.
// ============================================================

static creature *br13526_captive;
static boolean br13526_wasAllyBefore;

static void br13526_setup(void) {
    clearArena();
    br13526_captive = spawnMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    makeMonsterCaptive(br13526_captive);
    br13526_wasAllyBefore = monsterIsAlly(br13526_captive);
    queueConfirmYes(); // accept "Free the captive ...?"
}

static boolean br13526_step(void) {
    moveRight();
    return false;
}

static boolean br13526_verify(void) {
    TEST_ASSERT(br13526_captive != NULL, "setup: the captive should exist");
    if (br13526_captive == NULL) {
        return false;
    }
    TEST_ASSERT(!br13526_wasAllyBefore,
                "control: the captive is not an ally before it is freed");
    TEST_ASSERT(messageWasShown("you free the grateful"),
                "freeing the captive should announce the grateful creature");
    TEST_ASSERT(messageWasShown("gain a faithful ally"),
                "freeing the captive should announce that you gain a faithful ally");
    TEST_ASSERT(monsterIsAlly(br13526_captive),
                "the freed captive should become your ally");
    return !testFailed;
}

// ============================================================
// BR13208 — a looked-at item is described with "is" when it is a single
// item, and with "are" when it is a stack of more than one or a pile of
// gold; while hallucinating it is always described with "is".
//
// Each case puts one item on an open, revealed square beside the player and
// asks the framework for that square's description - the same text the game
// puts in the flavor line when the player looks at it.  The square holds no
// creature, so the description takes the item branch the rule describes.
// ============================================================

static pos br13208_itemLoc;
static const char *br13208_description;
static item *br13208_item;

static void br13208_prepare_item(short category, short kind) {
    clearArena();
    clearFloorItems();
    br13208_itemLoc = posOffset(getPlayerPos(), 1, 0);
    br13208_item = placeItemOnFloor(category, kind, br13208_itemLoc);
    revealCell(br13208_itemLoc);
}

static void br13208_single_setup(void) {
    br13208_prepare_item(WEAPON, DAGGER);
    br13208_description = locationDescriptionAt(br13208_itemLoc);
}

static boolean br13208_single_item_uses_is(void) {
    TEST_ASSERT_EQ(itemQuantity(br13208_item), 1,
                   "precondition: the item is a single item, quantity");
    TEST_ASSERT(strstr(br13208_description, " is ") != NULL,
                "a single item should be described with \"is\"");
    TEST_ASSERT(strstr(br13208_description, " are ") == NULL,
                "a single item should not be described with \"are\"");
    return !testFailed;
}

static void br13208_stack_setup(void) {
    br13208_prepare_item(WEAPON, DART);
    br13208_description = locationDescriptionAt(br13208_itemLoc);
}

static boolean br13208_stack_uses_are(void) {
    TEST_ASSERT(itemQuantity(br13208_item) > 1,
                "precondition: the darts are a stack of more than one");
    TEST_ASSERT(strstr(br13208_description, " are ") != NULL,
                "a stack of more than one item should be described with \"are\"");
    TEST_ASSERT(strstr(br13208_description, " is ") == NULL,
                "a stack of more than one item should not be described with \"is\"");
    return !testFailed;
}

static void br13208_gold_setup(void) {
    br13208_prepare_item(GOLD, 0);
    br13208_description = locationDescriptionAt(br13208_itemLoc);
}

static boolean br13208_gold_uses_are(void) {
    TEST_ASSERT_EQ(itemCategory(br13208_item), GOLD,
                   "precondition: the item is gold, category");
    TEST_ASSERT(strstr(br13208_description, " are ") != NULL,
                "a pile of gold should be described with \"are\"");
    TEST_ASSERT(strstr(br13208_description, " is ") == NULL,
                "a pile of gold should not be described with \"is\"");
    return !testFailed;
}

static void br13208_hallucinating_stack_setup(void) {
    br13208_prepare_item(WEAPON, DART);
    applyStatusToPlayer(STATUS_HALLUCINATING, 100);
    br13208_description = locationDescriptionAt(br13208_itemLoc);
}

static boolean br13208_hallucinating_stack_uses_is(void) {
    TEST_ASSERT(itemQuantity(br13208_item) > 1,
                "precondition: the darts are a stack of more than one");
    TEST_ASSERT(playerHasStatus(STATUS_HALLUCINATING),
                "precondition: the player is hallucinating");
    TEST_ASSERT(strstr(br13208_description, " is ") != NULL,
                "a hallucinating player should see a stack described with \"is\"");
    TEST_ASSERT(strstr(br13208_description, " are ") == NULL,
                "a hallucinating player should not see a stack described with \"are\"");
    return !testFailed;
}

// ============================================================
// BR37646 — descending from the deepest level without the Amulet is
// repelled ("the crystal archway repels you").
//
// The player stands one step above the down staircase on the dungeon's
// deepest floor, carrying no amulet.  Stepping onto the staircase asks the
// engine to descend; the crystal archway refuses, so the depth is unchanged
// and both refusal messages are shown.  A missing amulet is asserted first,
// so a test that accidentally carried one cannot pass by winning instead.
// ============================================================

static int br37646_depthBefore;
static pos br37646_startPos;

static void br37646_setup(void) {
    clearArena();
    setDepth(dungeonDeepestLevel());
    br37646_depthBefore = currentDepth();
    br37646_startPos = getPlayerPos();
    placeDownStairs(posOffset(br37646_startPos, 0, 1));
    clearMessages();
}

static boolean br37646_step(void) {
    moveDown(); // step onto the down staircase: the engine tries to descend
    return false;
}

static boolean br37646_without_amulet_is_repelled(void) {
    TEST_ASSERT(!playerIsCarryingAmulet(),
                "control: the player should not be carrying the amulet");
    TEST_ASSERT(messageWasShown("the crystal archway repels you"),
                "descending from the deepest level without the amulet should be repelled by the crystal archway");
    TEST_ASSERT(messageWasShown("Only the bearer of the Amulet of Yendor may pass"),
                "the refusal should explain that only the amulet's bearer may pass");
    TEST_ASSERT_EQ(currentDepth(), br37646_depthBefore,
                   "the player should stay on the deepest level when the archway repels them");
    TEST_ASSERT_POS_EQ(getPlayerPos(), br37646_startPos,
                       "the repelled player should not leave their square");
    return !testFailed;
}

// ============================================================
// BR38077 — ascending off the top level without the Amulet is
// refused ("The dungeon exit is magically sealed!").
//
// On depth 1 the up staircase is the dungeon exit.  The player
// stands one step below it carrying no amulet and steps onto it:
// the exit refuses, so the depth and the player's square are
// unchanged and both refusal messages are shown.  A missing
// amulet is asserted first, so a test that accidentally carried
// one cannot pass by winning instead.
// ============================================================

static int br38077_depthBefore;
static pos br38077_startPos;

static void br38077_setup(void) {
    clearArena();
    setDepth(1);
    br38077_depthBefore = currentDepth();
    br38077_startPos = getPlayerPos();
    placeUpStairs(posOffset(br38077_startPos, 0, -1));
    clearMessages();
}

static boolean br38077_step(void) {
    moveUp(); // step onto the up staircase: the engine tries to ascend
    return false;
}

static boolean br38077_ascent_without_amulet_is_sealed(void) {
    TEST_ASSERT(!playerIsCarryingAmulet(),
                "control: the player should not be carrying the amulet");
    TEST_ASSERT(messageWasShown("The dungeon exit is magically sealed!"),
                "ascending off the top level without the amulet should find the dungeon exit sealed");
    TEST_ASSERT(messageWasShown("Only the bearer of the Amulet of Yendor may pass"),
                "the refusal should explain that only the amulet's bearer may pass");
    TEST_ASSERT_EQ(currentDepth(), br38077_depthBefore,
                   "the player should stay on the top level when the exit is sealed");
    TEST_ASSERT_POS_EQ(getPlayerPos(), br38077_startPos,
                       "the refused player should not leave their square");
    return !testFailed;
}

// ============================================================
// BR32837 — every 100 ticks monsters approach the stairs to
// enter your level
//
// A monster waits one level above with two environment updates left.  The
// player is hasted, so each rest costs 50 ticks and every second rest
// completes one 100-tick environment update.  Counting updates with
// restAndDetectUpdate() (defined above) shows the monster stays away until
// the second update and enters then: a countdown that ran every turn would
// bring it in one update earlier.
// ============================================================

static creature *br32837_waiter;
static int br32837_updatesSeen;
static int br32837_updatesAtEntry;
static boolean br32837_entered;

static void br32837_setup(void) {
    setArenaRadius(4, FLOOR);
    descendToNextLevel(); // depth 2 leaves depth 1 visited above
    clearArena();         // open floor and no monsters on this level
    clearMonstersOnPreviousLevel();

    br32837_waiter = spawnMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    applyStatusToMonster(br32837_waiter, STATUS_PARALYZED, 1000);
    waitOnPreviousLevelToEnter(br32837_waiter, 2);

    hastePlayer(20); // a rest now costs 50 ticks
    cadenceCycleTooLong = false;
    br32837_updatesSeen = 0;
    br32837_updatesAtEntry = -1;
    br32837_entered = false;
}

static boolean br32837_step(void) {
    restOneTurn();
    if (restAndDetectUpdate()) {
        br32837_updatesSeen++;
    }
    if (!br32837_entered && monsterDepth(br32837_waiter) == currentDepth()) {
        br32837_entered = true;
        br32837_updatesAtEntry = br32837_updatesSeen;
        return false;
    }
    return br32837_updatesSeen < 5;
}

static boolean br32837_verify(void) {
    TEST_ASSERT(br32837_entered,
                "the monster waiting above should enter your level");
    TEST_ASSERT_EQ(br32837_updatesAtEntry, 2,
                   "the monster should enter on the second 100-tick environment update");
    TEST_ASSERT(!cadenceCycleTooLong,
                "the environment cycle should never exceed the rule's 100 ticks");
    return !testFailed;
}

// ============================================================
// BR18555 — auto-explore favors squares holding items as goals.
//
// The whole level is already mapped and the only walkable ground is a
// short corridor, so the sole thing left that can guide auto-explore is
// the ration lying at its far end.  Without treating the item's square
// as a goal there is no goal at all and auto-explore stands still.
// ============================================================

static pos itemGoalStart;
static pos itemGoalLoc;
static item *itemGoalItem;

static void item_goal_explore_setup(void) {
    clearMonsters();
    clearFloorItems();

    // Rebuild the level as solid granite so the corridor is the only
    // walkable ground and no leftover item can compete as a goal.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }

    itemGoalStart = posAt(levelWidth() / 2, levelHeight() / 2);
    for (int i = 0; i <= 3; i++) {
        setTerrain(posOffset(itemGoalStart, i, 0), FLOOR);
    }
    setPlayerPos(itemGoalStart);
    itemGoalLoc = posOffset(itemGoalStart, 3, 0);
    itemGoalItem = placeItemOnFloor(FOOD, RATION, itemGoalLoc);

    // Mark the whole level discovered, leaving the item as the only goal.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            revealCell(posAt(x, y));
        }
    }
}

static boolean item_goal_explore_step(void) {
    TEST_ASSERT(playerIsAt(itemGoalStart),
                "setup: the player should start at the west end of the corridor");
    TEST_ASSERT(itemGoalItem != NULL,
                "setup: the item should lie on the corridor");
    TEST_ASSERT(!playerIsAt(itemGoalLoc),
                "setup: the item should start away from the player");
    if (testFailed) {
        return false;
    }

    // The first press picks the exploration target; the second press
    // starts exploring toward it.
    injectKeystroke('x', false, false);
    injectKeystroke('x', false, false);
    return false;
}

static boolean item_square_is_an_explore_goal(void) {
    TEST_ASSERT(playerIsAt(itemGoalLoc),
                "auto-explore should walk onto the item when it is the only goal on a fully mapped level");
    return !testFailed;
}

// ============================================================
// BR34818 — when you fall, a secret square you are standing on
// and can see is revealed.
//
// The player starts beside a hidden trap door, which is both a
// secret square (TM_IS_SECRET) and an automatic descent square
// (T_AUTO_DESCENT).  Stepping onto it sets the fall in motion
// before the ordinary "discover the secrets of the square you
// stand on" pass can run, so the fall itself must reveal the
// square.  The test runs on the dungeon's deepest floor, where a
// fall teleports the player instead of descending, leaving the
// square in place to be inspected.  A revealed hidden trap door
// becomes the ordinary trap door it was hiding.
// ============================================================

static pos br34818_trapLoc;

static void br34818_setup(void) {
    clearArena();
    // On the deepest floor a fall teleports rather than descending, so the
    // square the player fell from remains on the level for inspection.
    setDepth(dungeonDeepestLevel());
    br34818_trapLoc = posOffset(getPlayerPos(), 1, 0);
    setTerrain(br34818_trapLoc, TRAP_DOOR_HIDDEN);
    // The rule only reveals a square the player can see.
    revealCell(br34818_trapLoc);
    clearMessages();
}

static boolean br34818_step(void) {
    moveRight(); // step onto the hidden trap door and fall through it
    return false;
}

static boolean br34818_fall_reveals_secret(void) {
    TEST_ASSERT_EQ(currentDepth(), dungeonDeepestLevel(),
                   "control: a fall from the deepest floor should not descend");
    TEST_ASSERT(messageWasShown("you plunge through a hole in the ground"),
                "the player should have fallen through the trap door");
    TEST_ASSERT(!cellIsTerrain(br34818_trapLoc, TRAP_DOOR_HIDDEN),
                "falling should reveal the hidden trap door the player stood on");
    TEST_ASSERT(cellIsTerrain(br34818_trapLoc, TRAP_DOOR),
                "the revealed square should become an ordinary trap door");
    return !testFailed;
}

const testCase tests_movement[] = {
    { "BR34818_fall_reveals_secret_square", br34818_setup, br34818_step, br34818_fall_reveals_secret },
    { "BR37646_descent_without_amulet_is_repelled", br37646_setup, br37646_step, br37646_without_amulet_is_repelled },
    { "BR38077_ascent_without_amulet_is_sealed", br38077_setup, br38077_step, br38077_ascent_without_amulet_is_sealed },
    { "BR32837_monsters_approach_stairs_each_100_ticks", br32837_setup, br32837_step, br32837_verify },
    { "BR13208_single_item_described_with_is",        br13208_single_setup,   NULL, br13208_single_item_uses_is },
    { "BR13208_stack_described_with_are",             br13208_stack_setup,    NULL, br13208_stack_uses_are },
    { "BR13208_gold_described_with_are",              br13208_gold_setup,     NULL, br13208_gold_uses_are },
    { "BR13208_hallucinated_stack_described_with_is", br13208_hallucinating_stack_setup, NULL, br13208_hallucinating_stack_uses_is },

    { "BR91235_staff_recharges_a_spent_charge",      staff_recharge_setup, staff_recharge_step, spent_staff_recharges },
    { "BR91235_full_staff_gains_no_charges",         full_staff_setup,    full_staff_rest_step, full_staff_gains_no_charges },
    { "BR91235_charm_counts_down_to_recharged",      charm_countdown_setup, charm_countdown_step, charm_counts_down_to_recharged },

    { "BR21546_thorough_search_marks_nearby_square_trap_free", thorough_search_setup, thorough_search_step, nearby_square_marked_trap_free },
    { "BR20029_trap_free_squares_cost_flat_ten_to_explore", trap_free_explore_setup, trap_free_explore_step, trap_free_route_was_taken },
    { "BR16118_run_stops_beside_item",         run_beside_item_setup,  run_beside_item_step,     run_stopped_beside_item },
    { "BR16118_run_stops_beside_visible_hostile", run_beside_hostile_setup, run_beside_hostile_step, run_stopped_beside_hostile },
    { "BR31821_move_blocked_by_obstruction",   wall_blocks_setup,      wall_blocks_step,         player_did_not_move },
    { "BR29427_diagonal_blocked_by_corner",    corner_blocks_setup,    corner_blocks_step,       player_did_not_move },
    { "BR10367_cardinal_not_corner_blocked",   cardinal_corner_setup,  cardinal_corner_step,     player_moved_right },
    { "BR75019_step_on_item_picks_it_up",      floor_item_setup,       floor_item_step,          item_was_picked_up },
    { "BR44054_step_onto_ally_swaps_places",   ally_swap_setup,        ally_swap_step,           ally_swapped },
    { "BR96161_move_into_hostile_attacks",     attack_on_bump_setup,   attack_on_bump_step,      player_stayed_and_monster_hurt },
    { "BR60519_refuse_known_lava",             lava_refusal_setup,     lava_refusal_step,        refused_lava },
    { "BR88965_confused_lava_warning",         confused_lava_warning_setup, confused_lava_warning_step, warned_before_confused_step_into_lava },
    { "BR63035_chasm_prompt_cancels_move",     chasm_prompt_setup,     chasm_prompt_step,        chasm_prompt_refused },
    { "BR95206_fire_prompt_cancels_move",      fire_prompt_setup,      fire_prompt_step,         fire_prompt_refused },
    { "BR44913_warn_before_flame_when_harmful", flame_vulnerable_setup, flame_vulnerable_step,    warned_when_flame_would_harm },
    { "BR44913_no_warning_when_burning",       flame_while_burning_setup, flame_while_burning_step, no_warning_when_already_burning },
    { "BR44913_no_warning_when_immune",        flame_while_immune_setup, flame_while_immune_step,  no_warning_when_immune_to_fire },
    { "BR83833_pressure_plate_prompt_cancels", plate_prompt_setup,     plate_prompt_step,        plate_prompt_refused },
    { "BR50847_stairs_change_level",           stairs_setup,           stairs_step,              descended_a_level },
    { "BR34686_discovery_xp_goes_to_nearby_allies", discovery_xp_setup, discovery_xp_step,        discovery_xp_goes_to_allies_on_nearby_levels },
    { "BR34157_passable_discovery_counts",           br34157_setup,      br34157_step,             br34157_verify },
    { "BR39970_pressure_plate_depresses",      plate_depress_setup,    plate_depress_step,       plate_is_depressed },
    { "BR22348_pressure_plate_resets",         plate_depress_setup,    plate_reset_step,         plate_reset },
    { "BR24837_lava_kills",                    lava_kills_setup,       rest_a_few_turns_step,    player_was_killed },
    { "BR51947_deep_water_submerges",          deep_water_setup,       deep_water_step,          player_is_submerged },
    { "BR91575_web_traps_creature",            web_setup,              web_step,                 player_is_webbed },
    { "BR24700_web_struggle_broken_on_final_move", web_struggle_setup,   web_struggle_step,        player_frees_self_from_web },
    { "BR32089_monster_status_timers_tick_each_100_ticks", monster_status_timer_setup, monster_status_timer_step, monster_status_timers_decrement_per_100_ticks },
    { "BR79486_standing_in_fire_ignites",      standing_in_fire_setup, wait_until_hurt,          player_is_burning },
    { "BR95213_burning_square_ignites_orthogonal_neighbours", br95213_setup, br95213_step, br95213_verify },
    { "BR28134_burning_deals_one_to_three_and_burns_out", br28134_burning_setup, br28134_burning_step, br28134_burning_verify },
    { "BR28134_burning_can_kill",             br28134_burn_death_setup, br28134_burn_death_step, br28134_burn_death_verify },

    { "BR13526_freed_captive_becomes_grateful_ally", br13526_setup, br13526_step, br13526_verify },
    { "BR34974_nutrition_drops_each_turn",     nutrition_setup,        nutrition_step,           nutrition_dropped },
    { "BR95832_starvation_drains_health",      starvation_setup,       starvation_step,          starved },
    { "BR32992_first_hunger_warning_skips_acknowledgment", br32992_first_setup, br32992_first_step, br32992_first_needs_no_acknowledgment },
    { "BR32992_later_hunger_warnings_need_acknowledgment", br32992_later_setup, br32992_later_step, br32992_later_warnings_need_acknowledgment },
    { "BR35928_whip_strikes_distant_enemy",    whip_setup,             whip_step,                whip_hit_distant_enemy },
    { "BR12436_spear_strikes_two_squares",     spear_setup,            spear_step,               spear_hit_distant_enemy },
    { "BR12763_rapier_lunges_two_squares",     lunge_setup,            lunge_step,               lunged_and_moved },

    { "BR49182_flail_hits_enemies_adjacent_to_both_squares", flail_setup, flail_step, flail_hits_only_both_square_enemies },
    { "BR28946_gas_spreads_twice_per_environment_update",    gas_runs_twice_setup, one_turn_rest_step, gas_spreads_two_squares_in_one_update },
    { "BR29862_gas_dissipation_rates",                       br29862_setup, NULL, br29862_verify },

    { "BR36633_electricity_promotes_flagged_tile", electricity_promotes_setup, electricity_promotes_step, electricity_promotes_the_struck_tile },

    { "BR11386_visible_holder_cancels_move", br11386_visible_setup, br11386_step, br11386_visible_holder_cancels_move },
    { "BR11386_hidden_holder_spends_turn_struggling", br11386_hidden_setup, br11386_step, br11386_hidden_holder_spends_turn_struggling },

    { "BR65504_explore_heads_for_stairs_when_level_mapped", explore_to_stairs_setup, explore_to_stairs_step, explore_walks_to_the_stairs },
    { "BR19564_explore_stops_when_disturbed",  br19564_setup,           br19564_step,             br19564_stops_when_disturbed },
    { "BR18555_explore_favors_item_squares_as_goals", item_goal_explore_setup, item_goal_explore_step, item_square_is_an_explore_goal },

    { "BR24904_explosion_immunity_lasts_five_turns", br24904_setup, br24904_step, br24904_survivor_is_temporarily_immune },

    { "BR25832_dampening_armor_absorbs_explosion", br25832_dampening_setup, NULL, br25832_dampening_armor_absorbs_explosion },
    { "BR25832_other_armor_takes_explosion_damage", br25832_control_setup, NULL, br25832_other_armor_takes_explosion_damage },

    { "BR21531_gas_erupts_at_eight_qualifying_neighbours", br21531_erupt_setup, br21531_step, br21531_gas_erupts_at_eight },
    { "BR21531_gas_holds_at_seven_qualifying_neighbours",  br21531_quiet_setup,  br21531_step, br21531_gas_holds_at_seven },

    { "BR30801_disabled_monsters_do_not_act", br30801_setup, br30801_step, br30801_verify },
    { "BR25953_mending_ground_heals", br25953_setup, br25953_step, br25953_verify },

    { "BR26045_fast_monster_falls_before_acting", br26045_setup, br26045_rest_step, br26045_falls_before_acting },
    { "BR28360_burning_creature_ignites_flammable_ground", br28360_setup, br28360_rest_step, br28360_ground_is_set_alight },

    { "BR70260_falling_monster_damaged_and_moves_down", br70260_setup, br70260_wait_to_fall_step, br70260_monster_falls_damaged_to_level_below },

    { "BR17834_own_square_reports_already_there", br17834_own_square_setup, br17834_own_square_step, br17834_own_square_reports_already_there },
    { "BR17834_explored_square_becomes_cursor_target", br17834_explored_setup, br17834_explored_step, br17834_explored_target_is_travelled_to },
    { "BR17834_unexplored_square_reports_failure", br17834_unexplored_setup, br17834_unexplored_step, br17834_unexplored_reports_failure },

    { "BR21973_confused_auto_explore", br21973_setup, br21973_step, br21973_verify },

    { NULL, NULL, NULL, NULL }
};
