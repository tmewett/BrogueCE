// Tests derived from rules/08-dungeon-generation.md.
//
// These cover the deterministic, headless-observable dungeon rules: the fixed
// level grid, the number and identity of its staircases, the granite the level
// is carved from, how the miner's light responds to darkness, water and light
// boosts, that lighting is recomputed from scratch, and the core pathfinding
// behaviours (eight-way movement, obstructions, corner-cutting, and the
// player's aversion to unexplored space).
//
// Omitted are everything about the generation process that leaves no
// deterministic, inspectable trace once the level exists: the order levels are
// built in, room/vault/lake/bridge shapes and placement, doorway and secret
// door selection, feature and auto-generator placement, stair-adjacency and
// waypoint bookkeeping, monster/item population, and the shade a tile is drawn
// in, all of which need the RNG stream, the drawing code, or a screen.

#include "TestFramework.h"

// The centre of the map, where there is always room to build a scene.
static pos sceneCentre(void) {
    return posAt(40, 14);
}

// ============================================================
// The level grid and its terrain
// ============================================================

static boolean level_is_79_by_29(void) {
    TEST_ASSERT_EQ(levelWidth(), 79, "a dungeon level should be 79 tiles wide");
    TEST_ASSERT_EQ(levelHeight(), 29, "a dungeon level should be 29 tiles high");
    return !testFailed;
}

static boolean carving_leaves_solid_granite(void) {
    TEST_ASSERT(countTerrain(GRANITE) > 0,
                "untouched areas of the level should remain solid granite");
    TEST_ASSERT(countTerrain(FLOOR) > 0,
                "the level should contain floor carved out of the granite");
    return !testFailed;
}

// ============================================================
// Staircases
// ============================================================

static boolean one_up_and_one_down_staircase(void) {
    // On depth 1 the up staircase is the dungeon exit (see BR87046).
    TEST_ASSERT_EQ(countTerrain(DUNGEON_EXIT), 1,
                   "depth 1 should have exactly one up staircase (the dungeon exit)");
    TEST_ASSERT_EQ(countTerrain(DOWN_STAIRS), 1,
                   "depth 1 should have exactly one down staircase");

    descendToNextLevel();
    TEST_ASSERT_EQ(currentDepth(), 2, "the player should now be on depth 2");
    TEST_ASSERT_EQ(countTerrain(UP_STAIRS), 1,
                   "an ordinary level should have exactly one up staircase");
    TEST_ASSERT_EQ(countTerrain(DOWN_STAIRS), 1,
                   "an ordinary level should have exactly one down staircase");
    return !testFailed;
}

static boolean depth1_up_staircase_is_the_dungeon_exit(void) {
    TEST_ASSERT_EQ(countTerrain(DUNGEON_EXIT), 1,
                   "the up staircase on depth 1 should be the dungeon exit");
    TEST_ASSERT_EQ(countTerrain(UP_STAIRS), 0,
                   "depth 1 should not hold an ordinary up staircase");
    return !testFailed;
}

// ============================================================
// Miner's light
// ============================================================

static boolean light_radius_floor_and_water_halving(void) {
    removeStatusFromPlayer(STATUS_DARKNESS);
    setPlayerInWater(false);
    setLightMultiplier(1);

    int dry = minersLightRadius();
    TEST_ASSERT(dry >= 200, "the light radius should be at least 2 tiles");

    applyStatusToPlayer(STATUS_DARKNESS, 100);
    int dark = minersLightRadius();
    TEST_ASSERT(dark >= 200, "the light radius should never fall below 2 tiles");
    removeStatusFromPlayer(STATUS_DARKNESS);

    setPlayerInWater(true);
    int water = minersLightRadius();
    TEST_ASSERT_EQ(water, dry / 2, "standing in water should halve the light radius");
    TEST_ASSERT(water >= 300, "the light radius in water should not fall below 3 tiles");
    return !testFailed;
}

static boolean darkness_retains_a_twentieth_of_light(void) {
    removeStatusFromPlayer(STATUS_DARKNESS);
    setPlayerInWater(false);
    setLightMultiplier(1);

    int normal = minersLightRadius();
    applyStatusToPlayer(STATUS_DARKNESS, 100);
    int dark = minersLightRadius();
    TEST_ASSERT(dark >= normal / 20,
                "darkness should leave at least a twentieth of the normal light");
    return !testFailed;
}

static boolean light_shrinks_in_darkness_and_water_and_grows_with_boosts(void) {
    removeStatusFromPlayer(STATUS_DARKNESS);
    setPlayerInWater(false);
    setLightMultiplier(1);
    int normal = minersLightRadius();

    applyStatusToPlayer(STATUS_DARKNESS, 100);
    int dark = minersLightRadius();
    TEST_ASSERT(dark < normal, "magical darkness should shrink the player's light");
    removeStatusFromPlayer(STATUS_DARKNESS);

    setPlayerInWater(true);
    int water = minersLightRadius();
    TEST_ASSERT(water < normal, "standing in water should shrink the player's light");
    setPlayerInWater(false);

    setLightMultiplier(2);
    int boosted = minersLightRadius();
    TEST_ASSERT(boosted > normal, "light-boosting effects should grow the player's light");
    return !testFailed;
}

static boolean lighting_is_recomputed_from_scratch(void) {
    // Strip the level down to plain granite so only the player's own light
    // matters, then carve a small pocket to stand in.
    for (int i = 0; i < levelWidth(); i++) {
        for (int j = 0; j < levelHeight(); j++) {
            setTerrain(posAt(i, j), GRANITE);
        }
    }
    pos there = getPlayerPos();
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(there, dx, dy), FLOOR);
        }
    }
    clearMonsters();

    recomputeLighting();
    int first = lightAt(there);
    recomputeLighting();
    int second = lightAt(there);

    TEST_ASSERT(first > 0, "the player's own light should illuminate their tile");
    TEST_ASSERT_EQ(second, first,
                   "recomputing the lighting should repaint it from scratch, not add to it");

    pos far = posAt(there.x < levelWidth() / 2 ? levelWidth() - 2 : 1,
                    there.y < levelHeight() / 2 ? levelHeight() - 2 : 1);
    TEST_ASSERT_EQ(lightAt(far), 0,
                   "a tile beyond the light should start each recompute unlit");
    return !testFailed;
}

// ============================================================
// Pathfinding
// ============================================================

static boolean pathfinding_uses_eight_directions(void) {
    pos a = sceneCentre();
    pos b = posOffset(a, 1, 1);

    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(a, dx, dy), GRANITE);
        }
    }
    // A 2x2 open block: the far tile is one diagonal step away.
    setTerrain(a, FLOOR);
    setTerrain(posOffset(a, 1, 0), FLOOR);
    setTerrain(posOffset(a, 0, 1), FLOOR);
    setTerrain(b, FLOOR);

    TEST_ASSERT_EQ(pathingDistanceBetween(a, b, 0), 1,
                   "a diagonal step should count as a single move (eight-way pathfinding)");
    return !testFailed;
}

static boolean obstructions_block_pathfinding(void) {
    pos middle = sceneCentre();
    pos left = posOffset(middle, -1, 0);
    pos right = posOffset(middle, 1, 0);

    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(middle, dx, dy), GRANITE);
        }
    }
    setTerrain(left, FLOOR);
    setTerrain(middle, GRANITE);
    setTerrain(right, FLOOR);

    TEST_ASSERT(pathingDistanceBetween(left, right, 0) >= 30000,
                "a tile marked as an obstruction should not be entered");
    return !testFailed;
}

static boolean diagonal_cannot_cut_past_a_wall_corner(void) {
    pos corner = sceneCentre();
    pos across = posOffset(corner, 1, 1);

    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(corner, dx, dy), GRANITE);
        }
    }
    setTerrain(corner, FLOOR);
    setTerrain(across, FLOOR);

    // The two orthogonal neighbours are walls, so the diagonal is a corner cut.
    TEST_ASSERT(pathingDistanceBetween(corner, across, 0) >= 30000,
                "a diagonal move should not be able to cut past a wall corner");

    // Opening the corners allows the diagonal step again.
    setTerrain(posOffset(corner, 1, 0), FLOOR);
    setTerrain(posOffset(corner, 0, 1), FLOOR);
    TEST_ASSERT_EQ(pathingDistanceBetween(corner, across, 0), 1,
                   "with the corners open the diagonal step is permitted");
    return !testFailed;
}

static boolean player_pathfinding_avoids_unexplored_tiles(void) {
    clearMonsters();
    pos from = posAt(20, 14);
    pos to = posAt(40, 14);

    // A straight corridor that the player has not yet discovered.
    for (int x = 20; x <= 40; x++) {
        setTerrain(posAt(x, 14), FLOOR);
    }
    TEST_ASSERT(playerPathingDistance(from, to) >= 30000,
                "the player should not path through tiles they have not discovered");

    for (int x = 20; x <= 40; x++) {
        revealCell(posAt(x, 14));
    }
    TEST_ASSERT_EQ(playerPathingDistance(from, to), 20,
                   "once discovered, the player paths straight down the corridor");
    return !testFailed;
}

// ============================================================

const testCase tests_dungeon[] = {
    { "BR71397_level_is_79_by_29",                       NULL, NULL, level_is_79_by_29 },
    { "BR51037_carving_leaves_solid_granite",            NULL, NULL, carving_leaves_solid_granite },
    { "BR69081_one_up_and_one_down_staircase",           NULL, NULL, one_up_and_one_down_staircase },
    { "BR87046_depth1_up_staircase_is_dungeon_exit",     NULL, NULL, depth1_up_staircase_is_the_dungeon_exit },
    { "BR94138_light_radius_floor_and_water_halving",    NULL, NULL, light_radius_floor_and_water_halving },
    { "BR23366_darkness_retains_a_twentieth_of_light",   NULL, NULL, darkness_retains_a_twentieth_of_light },
    { "BR23156_light_shrinks_and_grows_with_conditions", NULL, NULL, light_shrinks_in_darkness_and_water_and_grows_with_boosts },
    { "BR56856_lighting_is_recomputed_from_scratch",     NULL, NULL, lighting_is_recomputed_from_scratch },
    { "BR14015_pathfinding_uses_eight_directions",       NULL, NULL, pathfinding_uses_eight_directions },
    { "BR37348_obstructions_block_pathfinding",          NULL, NULL, obstructions_block_pathfinding },
    { "BR82699_diagonal_cannot_cut_past_wall_corner",    NULL, NULL, diagonal_cannot_cut_past_a_wall_corner },
    { "BR43009_player_pathfinding_avoids_unexplored",    NULL, NULL, player_pathfinding_avoids_unexplored_tiles },

    { NULL, NULL, NULL, NULL }
};
