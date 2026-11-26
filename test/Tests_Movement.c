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
// Shared setup helpers for the reach-weapon tests
// ============================================================

// generateItem() rolls a random enchant and possibly a runic — including
// mercy, which heals its victim back to full on hit — even when a specific
// weapon kind is requested, and givePlayerItem() ignores an enchant of 0.
// These tests need an exactly plain weapon so the strike's outcome is fixed.
static item *givePlainWeapon(short kind, short enchant) {
    item *theItem = givePlayerItem(WEAPON, kind, enchant);
    theItem->enchant1 = enchant;
    theItem->enchant2 = 0;
    theItem->flags &= ~(ITEM_RUNIC | ITEM_RUNIC_HINTED | ITEM_RUNIC_IDENTIFIED | ITEM_CURSED);
    return theItem;
}

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
    givePlainWeapon(WHIP, 0);
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
    givePlainWeapon(SPEAR, 0);
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
    theItem = givePlainWeapon(RAPIER, 10);
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
    givePlainWeapon(FLAIL, 0);
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

static item *spentStaff;
static int staffChargesBeforeZap;
static int staffChargesAfterZap;
static int rechargeRestTurns;

// Zap a monster with a fresh staff so the staff ends up one charge short of
// full, then clear the dungeon so nothing interferes while the charge returns.
static void staff_recharge_setup(void) {
    setArenaRadius(4, FLOOR);
    setPlayerNutrition(2000);
    spentStaff = givePlayerItem(STAFF, STAFF_LIGHTNING, 0);
    staffChargesBeforeZap = itemCharges(spentStaff);
    placeHostileMonster(MK_RAT, posOffset(player.loc, 3, 0));
    // A turn ends with a sidebar refresh, and auto-targeting reads the
    // sidebar's list of visible creatures to pick what to aim at.
    restOneTurn();
    queueKeystroke(RETURN_KEY, false, false);
    applyPackItem(STAFF, STAFF_LIGHTNING);
    clearMonsters();
    staffChargesAfterZap = itemCharges(spentStaff);
    rechargeRestTurns = 0;
}

static boolean staff_recharge_step(void) {
    restOneTurn();
    rechargeRestTurns++;
    return itemCharges(spentStaff) <= staffChargesAfterZap && rechargeRestTurns < 400;
}

static boolean spent_staff_recharges(void) {
    TEST_ASSERT_EQ(staffChargesAfterZap, staffChargesBeforeZap - 1,
                   "zapping should cost the staff one charge");
    TEST_ASSERT(itemCharges(spentStaff) > staffChargesAfterZap,
                "a staff that is short a charge should recharge over time");
    return !testFailed;
}

static item *fullStaff;
static int fullStaffCharges;
static int fullStaffRestTurns;

static void full_staff_setup(void) {
    setArenaRadius(4, FLOOR);
    setPlayerNutrition(2000);
    fullStaff = givePlayerItem(STAFF, STAFF_LIGHTNING, 0);
    fullStaffCharges = itemCharges(fullStaff);
    fullStaffRestTurns = 0;
}

static boolean full_staff_rest_step(void) {
    restOneTurn();
    fullStaffRestTurns++;
    return fullStaffRestTurns < 60;
}

static boolean full_staff_gains_no_charges(void) {
    TEST_ASSERT(fullStaffCharges > 0,
                "a freshly found staff should start out carrying charges");
    TEST_ASSERT_EQ(itemCharges(fullStaff), fullStaffCharges,
                   "a staff that is already full should not gain more charges");
    return !testFailed;
}

static item *countdownCharm;
static int charmChargesWhenUsed;
static int charmRestTurns;

static void charm_countdown_setup(void) {
    setArenaRadius(4, FLOOR);
    setPlayerNutrition(2000);
    countdownCharm = givePlayerItem(CHARM, CHARM_HASTE, 3);
    charmChargesWhenUsed = 0;
    applyPackItem(CHARM, CHARM_HASTE);
    charmChargesWhenUsed = itemCharges(countdownCharm);
    charmRestTurns = 0;
}

static boolean charm_countdown_step(void) {
    restOneTurn();
    charmRestTurns++;
    return itemCharges(countdownCharm) > 0 && charmRestTurns < 1000;
}

static boolean charm_counts_down_to_recharged(void) {
    TEST_ASSERT(charmChargesWhenUsed > 0,
                "using a charm should start its recharge countdown");
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

const testCase tests_movement[] = {
    { "BR91235_staff_recharges_a_spent_charge",      staff_recharge_setup, staff_recharge_step, spent_staff_recharges },
    { "BR91235_full_staff_gains_no_charges",         full_staff_setup,    full_staff_rest_step, full_staff_gains_no_charges },
    { "BR91235_charm_counts_down_to_recharged",      charm_countdown_setup, charm_countdown_step, charm_counts_down_to_recharged },

    { "BR21546_thorough_search_marks_nearby_square_trap_free", thorough_search_setup, thorough_search_step, nearby_square_marked_trap_free },
    { "BR20029_trap_free_squares_cost_flat_ten_to_explore", trap_free_explore_setup, trap_free_explore_step, trap_free_route_was_taken },
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
    { "BR83833_pressure_plate_prompt_cancels", plate_prompt_setup,     plate_prompt_step,        plate_prompt_refused },
    { "BR50847_stairs_change_level",           stairs_setup,           stairs_step,              descended_a_level },
    { "BR34686_discovery_xp_goes_to_nearby_allies", discovery_xp_setup, discovery_xp_step,        discovery_xp_goes_to_allies_on_nearby_levels },
    { "BR39970_pressure_plate_depresses",      plate_depress_setup,    plate_depress_step,       plate_is_depressed },
    { "BR22348_pressure_plate_resets",         plate_depress_setup,    plate_reset_step,         plate_reset },
    { "BR24837_lava_kills",                    lava_kills_setup,       rest_a_few_turns_step,    player_was_killed },
    { "BR51947_deep_water_submerges",          deep_water_setup,       deep_water_step,          player_is_submerged },
    { "BR91575_web_traps_creature",            web_setup,              web_step,                 player_is_webbed },
    { "BR24700_web_struggle_broken_on_final_move", web_struggle_setup,   web_struggle_step,        player_frees_self_from_web },
    { "BR32089_monster_status_timers_tick_each_100_ticks", monster_status_timer_setup, monster_status_timer_step, monster_status_timers_decrement_per_100_ticks },
    { "BR79486_standing_in_fire_ignites",      standing_in_fire_setup, wait_until_hurt,          player_is_burning },
    { "BR34974_nutrition_drops_each_turn",     nutrition_setup,        nutrition_step,           nutrition_dropped },
    { "BR95832_starvation_drains_health",      starvation_setup,       starvation_step,          starved },
    { "BR35928_whip_strikes_distant_enemy",    whip_setup,             whip_step,                whip_hit_distant_enemy },
    { "BR12436_spear_strikes_two_squares",     spear_setup,            spear_step,               spear_hit_distant_enemy },
    { "BR12763_rapier_lunges_two_squares",     lunge_setup,            lunge_step,               lunged_and_moved },

    { "BR49182_flail_hits_enemies_adjacent_to_both_squares", flail_setup, flail_step, flail_hits_only_both_square_enemies },
    { "BR28946_gas_spreads_twice_per_environment_update",    gas_runs_twice_setup, one_turn_rest_step, gas_spreads_two_squares_in_one_update },

    { "BR36633_electricity_promotes_flagged_tile", electricity_promotes_setup, electricity_promotes_step, electricity_promotes_the_struck_tile },

    { "BR11386_visible_holder_cancels_move", br11386_visible_setup, br11386_step, br11386_visible_holder_cancels_move },
    { "BR11386_hidden_holder_spends_turn_struggling", br11386_hidden_setup, br11386_step, br11386_hidden_holder_spends_turn_struggling },

    { "BR65504_explore_heads_for_stairs_when_level_mapped", explore_to_stairs_setup, explore_to_stairs_step, explore_walks_to_the_stairs },

    { "BR24904_explosion_immunity_lasts_five_turns", br24904_setup, br24904_step, br24904_survivor_is_temporarily_immune },

    { "BR21531_gas_erupts_at_eight_qualifying_neighbours", br21531_erupt_setup, br21531_step, br21531_gas_erupts_at_eight },
    { "BR21531_gas_holds_at_seven_qualifying_neighbours",  br21531_quiet_setup,  br21531_step, br21531_gas_holds_at_seven },

    { "BR30801_disabled_monsters_do_not_act", br30801_setup, br30801_step, br30801_verify },
    { "BR25953_mending_ground_heals", br25953_setup, br25953_step, br25953_verify },

    { NULL, NULL, NULL, NULL }
};
