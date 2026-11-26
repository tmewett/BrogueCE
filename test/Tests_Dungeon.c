// Tests derived from rules/08-dungeon-generation.md.
//
// These cover the deterministic, headless-observable dungeon rules: the fixed
// level grid, the number and identity of its staircases, the granite the level
// is carved from, how the miner's light responds to darkness, water and light
// boosts, that lighting is recomputed from scratch, and the core pathfinding
// behaviours (eight-way movement, obstructions, corner-cutting, and the
// player's aversion to unexplored space), where a distance map places a
// dropped item, and the gas volume a
// dungeon feature gives the tile it is set on.  Generated levels are also
// checked for connectedness: no floor may be stranded by the blocking
// terrain the generator places, and for the shape of chunky rooms, which
// must come out exactly as a chained union of radius-2 discs.
//
// Omitted are everything about the generation process that leaves no
// deterministic, inspectable trace once the level exists: the order levels are
// built in, the shapes of rooms other than chunky rooms and the three cave
// size classes (the number of rooms placed on a level is still checked
// against its generation cap, and how often rooms get hallways is checked
// against depth), vault/lake shapes and placement, doorway and secret
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
    TEST_ASSERT(lightAt(far) == 0,
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
// BR33256 — damage-immune stationary monsters are always avoided
// as obstacles during pathfinding.
//
// A full-height wall splits the level in two with a single gap
// tile, so routing from one side to the other exercises exactly
// that tile.  Three monsters take turns standing in the gap: a
// goblin totem (stationary but breakable — the route stays open,
// showing that mere presence is not what blocks it), a mirrored
// totem (stationary AND damage-immune — the route must close),
// and the Warden of Yendor (damage-immune but mobile — the route
// opens again).  Only the combination the rule names is avoided.
// ============================================================

static boolean damage_immune_stationary_monsters_are_pathfinding_obstacles(void) {
    pos gap = sceneCentre();
    pos left = posOffset(gap, -2, 0);
    pos right = posOffset(gap, 2, 0);

    clearMonsters();
    for (int y = 0; y < levelHeight(); y++) {
        setTerrain(posAt(gap.x, y), GRANITE);
    }
    for (int x = left.x; x <= right.x; x++) {
        setTerrain(posAt(x, gap.y), FLOOR);
    }

    TEST_ASSERT_EQ(pathingDistanceBetween(left, right, 0), 4,
                   "control: the gap in the wall is the only route across");
    creature *blocker = spawnMonster(MK_GOBLIN_TOTEM, gap);
    TEST_ASSERT(blocker != NULL, "goblin totem spawned in the gap");
    TEST_ASSERT_EQ(pathingDistanceBetween(left, right, 0), 4,
                   "a stationary but damageable monster should not block the route");

    clearMonsters();
    blocker = spawnMonster(MK_MIRRORED_TOTEM, gap);
    TEST_ASSERT(blocker != NULL, "mirrored totem spawned in the gap");
    TEST_ASSERT(pathingDistanceBetween(left, right, 0) >= 30000,
                "a damage-immune stationary monster must be avoided: the route is closed");

    clearMonsters();
    blocker = spawnMonster(MK_WARDEN_OF_YENDOR, gap);
    TEST_ASSERT(blocker != NULL, "warden spawned in the gap");
    TEST_ASSERT_EQ(pathingDistanceBetween(left, right, 0), 4,
                   "a damage-immune monster that is not stationary is not avoided");
    return !testFailed;
}

// ============================================================
// BR79876 — a gas dungeon feature set on a tile adds its start
// probability to that tile's gas volume
// ============================================================

static pos singleFeatureTile;
static pos stackedFeatureTile;

// Wall the given tile in on all eight sides so its gas cannot spread away;
// all it can do is dissipate, at a rate that depends on how much of it there
// was to begin with.
static void sealGasTile(pos centre) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx != 0 || dy != 0) {
                setTerrain(posOffset(centre, dx, dy), GRANITE);
            }
        }
    }
}

static void gas_volume_setup(void) {
    pos centre = sceneCentre();
    setTerrain(centre, FLOOR);
    setPlayerPos(centre);
    setArenaRadius(5, FLOOR);

    singleFeatureTile = posOffset(centre, -3, 0);
    stackedFeatureTile = posOffset(centre, 3, 0);
    sealGasTile(singleFeatureTile);
    sealGasTile(stackedFeatureTile);

    // The same gas feature once, and five times on the same tile.  If each
    // set adds its start probability to the tile's gas volume, the second
    // tile holds five times as much gas as the first.
    spawnFeature(singleFeatureTile, DF_VENT_SPEW_POISON_GAS);
    for (int i = 0; i < 5; i++) {
        spawnFeature(stackedFeatureTile, DF_VENT_SPEW_POISON_GAS);
    }
}

// Give the gas two hundred turns of environment updates to dissipate away.
static boolean gas_dissipates_step(void) {
    for (int i = 0; i < 200; i++) {
        restOneTurn();
    }
    return false;
}

static boolean stacked_gas_volume_outlasts_single(void) {
    TEST_ASSERT(!cellHasGas(singleFeatureTile),
                "the gas from one small feature should have dissipated entirely after 200 turns");
    TEST_ASSERT(cellHasGas(stackedFeatureTile),
                "five applications of the same feature should have added their start "
                "probabilities together, leaving five times the gas to dissipate");
    return !testFailed;
}

// ============================================================
// BR26376 — a vault purges its interior of interesting terrain,
// hazards, or liquids
// ============================================================

// Count the vault interiors on the current level, how many of their tiles hold
// a liquid, and how many liquid tiles the level holds in total.
static void scanVaultLiquids(int *vaultCells, int *vaultLiquidCells, int *levelLiquidCells) {
    *vaultCells = 0;
    *vaultLiquidCells = 0;
    *levelLiquidCells = 0;
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos p = posAt(x, y);
            if (cellHasLiquid(p)) {
                (*levelLiquidCells)++;
            }
            if (cellIsVaultInterior(p)) {
                (*vaultCells)++;
                if (cellHasLiquid(p)) {
                    (*vaultLiquidCells)++;
                }
            }
        }
    }
}

// The fixed test seed builds depths 2 through 6 with vaults standing in and
// around the level's lakes, so a purged vault interior is an island of dry
// floor while the level around it keeps its water and lava.  Depth 1 is
// skipped because its vaults hold liquid that their own features placed after
// the purge ran.
static boolean vaults_purge_liquids_from_their_interiors(void) {
    for (int depth = 2; depth <= 6; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }

        int vaultCells, vaultLiquid, levelLiquid;
        scanVaultLiquids(&vaultCells, &vaultLiquid, &levelLiquid);

        TEST_ASSERT(vaultCells > 0,
                    "the level should contain at least one vault interior");
        TEST_ASSERT(levelLiquid > 0,
                    "the level should hold liquid (otherwise a dry vault proves nothing)");
        TEST_ASSERT_EQ(vaultLiquid, 0,
                       "a vault interior should be purged of liquids while the level around it holds liquid");
    }
    return !testFailed;
}

// ============================================================
// BR26268 — A hallway bolted onto a room is a straight run 5-15 tiles long
// when horizontal or 2-9 tiles long when vertical, with a 15% chance of an
// oblique exit at its far end.
//
// On the finished level a hallway shows up as a straight, one-tile-wide run
// of floor walled on both flanks, open into a room at one end and closed by a
// doorway at the other.  Only such door-terminated runs are measured: a door
// site that did not become a door merges the hallway into the floor beyond it
// and no longer bounds a clean run.  The 15% oblique-exit chance needs the RNG
// stream and is not checked here.
// ============================================================

static boolean hallFloor(short t) { return t == FLOOR; }
static boolean hallDoor(short t) {
    return t == DOOR || t == OPEN_DOOR || t == SECRET_DOOR || t == LOCKED_DOOR;
}

// Does the floor tile at p, sitting at the end of a run, open sideways (across
// the run's axis) into another floor tile — i.e. it borders a room rather than
// another one-tile-wide passage?  `horizontal` is the run's orientation, so the
// sideways look crosses the other axis.  p must already be known in range.
static boolean sidewaysIsRoom(pos p, boolean horizontal) {
    if (horizontal) {
        short up = (p.y > 0) ? terrainAt(posAt(p.x, p.y - 1)) : NOTHING;
        short dn = (p.y + 1 < levelHeight()) ? terrainAt(posAt(p.x, p.y + 1)) : NOTHING;
        return hallFloor(up) || hallFloor(dn);
    }
    short lf = (p.x > 0) ? terrainAt(posAt(p.x - 1, p.y)) : NOTHING;
    short rt = (p.x + 1 < levelWidth()) ? terrainAt(posAt(p.x + 1, p.y)) : NOTHING;
    return hallFloor(lf) || hallFloor(rt);
}

// Fold every straight, walled, one-tile-wide floor run on the level that runs
// from a room into a doorway into the running counts and per-orientation
// maxima.
static void scanHallways(int *hCount, int *vCount, int *maxH, int *maxV) {
    int W = levelWidth(), H = levelHeight();

    // Horizontal runs: floor whose up AND down neighbours are both non-floor.
    for (int y = 0; y < H; y++) {
        int start = -1;
        for (int x = 0; x <= W; x++) {
            boolean corr = false;
            if (x < W) {
                corr = hallFloor(terrainAt(posAt(x, y)))
                    && (y - 1 < 0 || !hallFloor(terrainAt(posAt(x, y - 1))))
                    && (y + 1 >= H || !hallFloor(terrainAt(posAt(x, y + 1))));
            }
            if (corr) {
                if (start < 0) start = x;
            } else if (start >= 0) {
                int len = x - start;
                short leftC  = (start - 1 >= 0) ? terrainAt(posAt(start - 1, y)) : GRANITE;
                short rightC = (x < W) ? terrainAt(posAt(x, y)) : GRANITE;
                boolean roomLeft  = hallFloor(leftC)  && sidewaysIsRoom(posAt(start - 1, y), true);
                boolean roomRight = hallFloor(rightC) && sidewaysIsRoom(posAt(x, y), true);
                if ((hallDoor(leftC) && roomRight) || (hallDoor(rightC) && roomLeft)) {
                    (*hCount)++;
                    if (len > *maxH) *maxH = len;
                }
                start = -1;
            }
        }
    }

    // Vertical runs: floor whose left AND right neighbours are both non-floor.
    for (int x = 0; x < W; x++) {
        int start = -1;
        for (int y = 0; y <= H; y++) {
            boolean corr = false;
            if (y < H) {
                corr = hallFloor(terrainAt(posAt(x, y)))
                    && (x - 1 < 0 || !hallFloor(terrainAt(posAt(x - 1, y))))
                    && (x + 1 >= W || !hallFloor(terrainAt(posAt(x + 1, y))));
            }
            if (corr) {
                if (start < 0) start = y;
            } else if (start >= 0) {
                int len = y - start;
                short upC = (start - 1 >= 0) ? terrainAt(posAt(x, start - 1)) : GRANITE;
                short dnC = (y < H) ? terrainAt(posAt(x, y)) : GRANITE;
                boolean roomUp = hallFloor(upC) && sidewaysIsRoom(posAt(x, start - 1), false);
                boolean roomDn = hallFloor(dnC) && sidewaysIsRoom(posAt(x, y), false);
                if ((hallDoor(upC) && roomDn) || (hallDoor(dnC) && roomUp)) {
                    (*vCount)++;
                    if (len > *maxV) *maxV = len;
                }
                start = -1;
            }
        }
    }
}

// Generate depths 1..26 and check that hallways are straight runs whose
// lengths stay inside the rule's bounds and reach its floor.
static boolean hallways_respect_length_bounds(void) {
    int hCount = 0, vCount = 0, maxH = 0, maxV = 0;
    for (int depth = 1; depth <= 26; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        scanHallways(&hCount, &vCount, &maxH, &maxV);
    }

    TEST_ASSERT(hCount > 0, "levels should contain horizontal room-to-door hallways");
    TEST_ASSERT(vCount > 0, "levels should contain vertical room-to-door hallways");
    TEST_ASSERT(maxH <= 15, "no horizontal hallway should run longer than 15 tiles");
    TEST_ASSERT(maxV <= 9, "no vertical hallway should run longer than 9 tiles");
    TEST_ASSERT(maxH >= 5, "the longest horizontal hallway should reach the 5-tile floor");
    TEST_ASSERT(maxV >= 2, "the longest vertical hallway should reach the 2-tile floor");
    return !testFailed;
}

// ============================================================
// BR68830 — distance maps place dropped items on the nearest
// reachable valid tile
// ============================================================

static creature *br68830_captive;
static pos br68830_expected;

static void br68830_drop_setup(void) {
    clearMonsters();
    removeFloorFood();

    pos p = getPlayerPos();

    // A sealed one-tile-wide pocket east of the player: three passable tiles
    // (p+1, p+2, p+3) walled off from the rest of the level, so the only
    // route out of the pocket runs west through them.
    for (int dx = 1; dx <= 4; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(p, dx, dy), GRANITE);
        }
    }
    setTerrain(posOffset(p, 1, 0), FLOOR);
    setTerrain(posOffset(p, 2, 0), FLOOR);
    setTerrain(posOffset(p, 3, 0), FLOOR);

    // The captive's own square (p+3) and the square it would have to route
    // through (p+2) already hold items, so neither can be chosen; p+1 is the
    // nearest reachable valid tile.
    placeItemOnFloor(GOLD, 0, posOffset(p, 3, 0));
    placeItemOnFloor(GOLD, 0, posOffset(p, 2, 0));

    br68830_captive = spawnMonster(MK_RAT, posOffset(p, 3, 0));
    giveMonsterItem(br68830_captive, FOOD, RATION);
    makeMonsterCaptive(br68830_captive);

    br68830_expected = posOffset(p, 1, 0);
}

static boolean br68830_drop_step(void) {
    // A captive drops whatever it carries when it takes its turn.
    monsterTakesTurn(br68830_captive);
    return false;
}

static boolean dropped_item_lands_on_nearest_reachable_tile(void) {
    item *dropped = findFloorItem(FOOD, RATION);
    TEST_ASSERT(dropped != NULL,
                "the captive should drop the ration it carries");
    if (dropped != NULL) {
        TEST_ASSERT_POS_EQ(itemPosition(dropped), br68830_expected,
                           "the dropped item should land on the nearest reachable valid tile");
    }
    return !testFailed;
}

// ============================================================
// BR93781 — bridges are built across bridgable gaps (chasms) only
// when the gap is at least 4 tiles long, the run passes beside at
// least one open tile, both ends reach open land, and the walking
// route it shortens is long enough for the depth.
//
// On the finished level a bridge is a straight run of bridge tiles
// with open land at both ends, so every run found while descending
// depths 1-26 is measured against the rule.  The shortcut ratio is
// depth-dependent and partly random, so only its depth-free lower
// bound can be re-checked after the fact: the depth term never
// subtracts and the random multiplier is at least 1, which forces
// every horizontal bridge over 200 and every vertical one over 500
// hundredths of its span.
// ============================================================

#define BRIDGE_RUN_CAP 128

typedef struct {
    short x1, y1, x2, y2;   // bridge tiles, from one end of the run to the other
    boolean horizontal;
} bridgeRun;

// An end of a bridge must be open land: nothing in the way, no
// bridgable gap to fall into, and outside any machine.  finishDoors
// may seal the doorway an end tile holds after the bridge is already
// built, so a secret door counts as the open land it was placed on.
static void checkBridgeEnd(pos end) {
    TEST_ASSERT(cellIsTerrain(end, SECRET_DOOR) || !cellObstructsPassage(end),
                "a bridge must end on open land");
    TEST_ASSERT(!cellIsTerrain(end, CHASM) && !cellIsTerrain(end, HOLE)
                && !cellIsTerrain(end, TRAP_DOOR) && !cellIsTerrain(end, TRAP_DOOR_HIDDEN),
                "a bridge must end on land, not in another gap");
    TEST_ASSERT(!cellIsVaultInterior(end),
                "a bridge must end outside any machine");
}

// Validate one run's placement and record it for the shortcut check below.
// Returns the updated record count.
static int recordBridgeRun(bridgeRun *runs, int cap, int count,
                           short x1, short y1, short x2, short y2,
                           boolean horizontal) {
    short span = horizontal ? (short)(x2 - x1 + 2) : (short)(y2 - y1 + 2);
    TEST_ASSERT(span >= 4, "a bridge must span a gap of at least 4 tiles");

    if (horizontal) {
        checkBridgeEnd(posAt(x1 - 1, y1));
        checkBridgeEnd(posAt(x2 + 1, y1));
    } else {
        checkBridgeEnd(posAt(x1, y1 - 1));
        checkBridgeEnd(posAt(x1, y2 + 1));
    }

    // At least one tile of the run must have an open tile beside it.
    boolean exposed = false;
    if (horizontal) {
        for (short t = x1; t <= x2 && !exposed; t++) {
            exposed = !cellObstructsPassage(posAt(t, y1 - 1))
                   || !cellObstructsPassage(posAt(t, y1 + 1));
        }
    } else {
        for (short t = y1; t <= y2 && !exposed; t++) {
            exposed = !cellObstructsPassage(posAt(x1 - 1, t))
                   || !cellObstructsPassage(posAt(x1 + 1, t));
        }
    }
    TEST_ASSERT(exposed, "a bridge must run beside at least one open tile");

    if (count < cap) {
        runs[count].x1 = x1;
        runs[count].y1 = y1;
        runs[count].x2 = x2;
        runs[count].y2 = y2;
        runs[count].horizontal = horizontal;
        count++;
    }
    return count;
}

// Find every maximal straight run of bridge tiles on the current level,
// validate each run's placement, and record up to cap runs.  A run that
// stands alone in both directions is recorded too: generation only ever
// builds runs of three tiles or more, so a lone tile is itself a violation
// that the span check catches.
static int scanBridgeRuns(bridgeRun *runs, int cap, int count) {
    static boolean seen[79][29];
    int W = levelWidth(), H = levelHeight();
    memset(seen, 0, sizeof(seen));

    for (int y = 1; y <= H - 2; y++) {
        for (int x = 1; x <= W - 2; x++) {
            if (seen[x][y] || !cellIsTerrain(posAt(x, y), BRIDGE)) {
                continue;
            }

            short x1 = x, x2 = x;
            while (x1 > 1 && cellIsTerrain(posAt(x1 - 1, y), BRIDGE)) x1--;
            while (x2 < W - 2 && cellIsTerrain(posAt(x2 + 1, y), BRIDGE)) x2++;
            boolean horizontal = (x2 > x1);
            short y1 = y, y2 = y;
            if (!horizontal) {
                while (y1 > 1 && cellIsTerrain(posAt(x, y1 - 1), BRIDGE)) y1--;
                while (y2 < H - 2 && cellIsTerrain(posAt(x, y2 + 1), BRIDGE)) y2++;
            }
            if (horizontal) {
                for (short t = x1; t <= x2; t++) seen[t][y] = 1;
            } else {
                for (short t = y1; t <= y2; t++) seen[x][t] = 1;
            }
            count = recordBridgeRun(runs, cap, count, x1, y1, x2, y2, horizontal);
        }
    }
    return count;
}

// Rebuild the walking route the bridge replaces: block the run with chasm
// again and route between the two land ends exactly as generation did.
// Generation demanded 100 * route / span > a ratio built from
// 100 + (100 horizontal | 400 vertical + a non-negative depth term) times a
// multiplier of at least 1, so the route must clear 200 (horizontal) or 500
// (vertical) hundredths of the span even at the shallowest depth.
static void checkBridgeShortcut(bridgeRun run) {
    int span, detour, ratio, required;

    if (run.horizontal) {
        for (short x = run.x1; x <= run.x2; x++) {
            setTerrain(posAt(x, run.y1), CHASM);
        }
        detour = pathingDistanceBetween(posAt(run.x1 - 1, run.y1),
                                        posAt(run.x2 + 1, run.y1),
                                        T_PATHING_BLOCKER);
        for (short x = run.x1; x <= run.x2; x++) {
            setTerrain(posAt(x, run.y1), BRIDGE);
        }
        span = run.x2 - run.x1 + 2;
        required = 200;
    } else {
        for (short y = run.y1; y <= run.y2; y++) {
            setTerrain(posAt(run.x1, y), CHASM);
        }
        detour = pathingDistanceBetween(posAt(run.x1, run.y1 - 1),
                                        posAt(run.x1, run.y2 + 1),
                                        T_PATHING_BLOCKER);
        for (short y = run.y1; y <= run.y2; y++) {
            setTerrain(posAt(run.x1, y), BRIDGE);
        }
        span = run.y2 - run.y1 + 2;
        required = 500;
    }

    ratio = 100 * detour / span;
    if (ratio <= required) {
        char buf[200];
        sprintf(buf, "a bridge must shorten the walking route enough for its depth (shortcut ratio %d must exceed %d)",
                ratio, required);
        testFail(buf);
    }
}

// Generate depths 1-26 and check every bridge they hold against the rule.
// The shortcut check rewrites terrain, so it runs while its own level is
// still the current one.
static boolean bridges_meet_placement_conditions(void) {
    static bridgeRun runs[BRIDGE_RUN_CAP];
    int found = 0;

    for (int depth = 1; depth <= 26; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        int levelStart = found;
        found = levelStart + scanBridgeRuns(runs + levelStart, BRIDGE_RUN_CAP - levelStart, 0);
        for (int i = levelStart; i < found; i++) {
            checkBridgeShortcut(runs[i]);
        }
    }

    TEST_ASSERT(found > 0, "levels should contain at least one bridge");
    return !testFailed;
}

// ============================================================
// BR44816 — before placing blocking terrain the game flood-fills the
// regions around it and refuses a placement that would cut a region off
// ============================================================

// Generate a batch of levels and confirm that none of their dry floor is
// stranded: every walkable floor tile must sit in the region that holds the
// player's starting tile, which is exactly the guarantee the flood-fill
// gives each time the generator weighs a blocking placement.
static boolean br44816_levels_stay_connected(void) {
    for (int depth = 1; depth <= 6; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        TEST_ASSERT(countTerrain(FLOOR) > 100,
                    "the level should carve out a substantial walkable area");
        TEST_ASSERT_EQ(countUnreachableFloorFrom(getPlayerPos()), 0,
                       "no floor tile may be cut off from the player's region");
        if (testFailed) {
            return false;
        }
    }

    // Control: the scan has to notice a region that really is cut off.  Wall
    // an interior floor tile in on all four sides and it must be reported.
    clearMonsters();
    pos start = getPlayerPos();
    pos pocket = posAt(-1, -1);
    for (int x = 4; x < levelWidth() - 4 && pocket.x < 0; x++) {
        for (int y = 4; y < levelHeight() - 4 && pocket.x < 0; y++) {
            if (terrainAt(posAt(x, y)) == FLOOR
                && !cellHasLiquid(posAt(x, y))
                && (x - start.x > 3 || start.x - x > 3)
                && (y - start.y > 3 || start.y - y > 3)) {
                pocket = posAt(x, y);
            }
        }
    }
    TEST_ASSERT(pocket.x >= 0,
                "the level should hold an interior floor tile to wall in");
    setTerrain(posOffset(pocket, -1, 0), GRANITE);
    setTerrain(posOffset(pocket, 1, 0), GRANITE);
    setTerrain(posOffset(pocket, 0, -1), GRANITE);
    setTerrain(posOffset(pocket, 0, 1), GRANITE);
    TEST_ASSERT(countUnreachableFloorFrom(start) > 0,
                "control: floor walled in on all sides must be reported as cut off");
    return !testFailed;
}

// ============================================================
// BR24979 — lakes are generated on canvases that begin at 30 x
// 15 and shrink by 2 x 1 each round down to a minimum of 20 x 10.
//
// The finished level shows the union of the round blobs, the
// chains the filler welds between blobs within 4 tiles, and the
// shallow wreath around each — so the canvas bounds surface as
// bounds on each connected body of liquid.  A bound derived from
// first principles alone would have to allow six chained blobs
// and reach the level's own width, which would prove nothing;
// instead the bounds pin the fixed test seed's observed maxima
// (51 x 27 across depths 1-26) with a small margin, staying
// strictly below the 79 x 29 level so a canvas that grows past
// the rule's 30 x 15 start, or merge behaviour that welds bodies
// together further, pushes some body over the line.  The levels
// must also actually hold lakes, and the largest body must reach
// at least the minimum 20-wide canvas, for the bound to prove
// anything.
// ============================================================

#define LAKE_BODY_MAX_WIDTH  55
#define LAKE_BODY_MAX_HEIGHT 28

// Find every 8-connected body of liquid on the level and fold its bounding
// box into the running maxima; also count the liquid tiles themselves.
static void scanLiquidBodies(int *liquidCells, int *bodyCount, int *maxW, int *maxH) {
    int W = levelWidth(), H = levelHeight();
    static boolean seen[79][29];
    pos queue[79 * 29];

    for (int x = 0; x < W; x++) {
        for (int y = 0; y < H; y++) {
            seen[x][y] = false;
        }
    }

    for (int sx = 0; sx < W; sx++) {
        for (int sy = 0; sy < H; sy++) {
            if (seen[sx][sy] || !cellHasLiquid(posAt(sx, sy))) {
                continue;
            }
            (*bodyCount)++;
            int head = 0, tail = 0;
            int minX = sx, maxX = sx, minY = sy, maxY = sy;
            seen[sx][sy] = true;
            queue[tail++] = posAt(sx, sy);
            while (head < tail) {
                pos p = queue[head++];
                (*liquidCells)++;
                if (p.x < minX) minX = p.x;
                if (p.x > maxX) maxX = p.x;
                if (p.y < minY) minY = p.y;
                if (p.y > maxY) maxY = p.y;
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) {
                            continue;
                        }
                        int nx = p.x + dx, ny = p.y + dy;
                        if (nx >= 0 && nx < W && ny >= 0 && ny < H
                            && !seen[nx][ny] && cellHasLiquid(posAt(nx, ny))) {
                            seen[nx][ny] = true;
                            queue[tail++] = posAt(nx, ny);
                        }
                    }
                }
            }
            int w = maxX - minX + 1, h = maxY - minY + 1;
            if (w > *maxW) *maxW = w;
            if (h > *maxH) *maxH = h;
        }
    }
}

// Generate depths 1 through 26 and check every lake body against the canvas
// bounds, with a control that the levels hold lakes at all.
static boolean lake_bodies_fit_the_generation_canvas(void) {
    int liquidCells = 0, bodyCount = 0, maxW = 0, maxH = 0;

    for (int depth = 1; depth <= 26; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        scanLiquidBodies(&liquidCells, &bodyCount, &maxW, &maxH);
    }

    TEST_ASSERT(liquidCells > 0,
                "the generated levels should hold liquid lakes (otherwise the bound proves nothing)");
    TEST_ASSERT(maxW >= 20,
                "the largest lake should reach at least the minimum canvas width of 20");
    TEST_ASSERT(maxW <= LAKE_BODY_MAX_WIDTH,
                "no lake body should outgrow its generation canvas plus merge reach and wreath");
    TEST_ASSERT(maxH <= LAKE_BODY_MAX_HEIGHT,
                "no lake body should outgrow its generation canvas height");
    return !testFailed;
}

// ============================================================
// BR88382 — Chunky rooms are grown by overlapping 2-8 circles of
// radius 2 outward from the map centre.
//
// A chunky room is the union of a seed disc plus 2-8 more discs of
// radius 2, each new centre lying inside the union placed so far, so
// the test looks for a floor region that is EXACTLY such a chained
// union of 3-9 discs.  Nothing else on a finished level can be:
// a rectangle or cross corner cannot be covered by a disc that stays
// inside the shape, hallways are one tile wide (no disc fits), the
// junctions between rooms that ended up merged are unreachable, and
// caves and caverns are irregular.  A plain circular room of radius 3
// is itself a union of radius-2 discs, so those are recognised and
// excluded separately.  The cover pins the radius, the circle count
// and the overlapping outward growth the rule describes: change any
// of them and no region on any level matches any more.
// ============================================================

#define BR88382_AREA (79 * 29)
#define BR88382_MAX_DISCS 9

// A radius-2 disc is every offset with dx*dx + dy*dy < 6 (the generator's
// circle rule): 21 cells.
#define BR88382_DISC_CELLS 21
static const signed char br88382_dx[BR88382_DISC_CELLS] = {
    0,  1, -1,  0,  0,  1,  1, -1, -1,  2, -2,  0,  0,  2,  2, -2, -2,  1, -1,  1, -1
};
static const signed char br88382_dy[BR88382_DISC_CELLS] = {
    0,  0,  0,  1, -1,  1, -1,  1, -1,  0,  0,  2, -2,  1, -1,  1, -1,  2,  2, -2, -2
};

static int br88382_W, br88382_H;
static unsigned char br88382_seen[BR88382_AREA];     // floor cells already collected
static unsigned char br88382_region[BR88382_AREA];   // cells of the region under test
static unsigned char br88382_covered[BR88382_AREA];  // cells reached by the discs placed so far
static unsigned char br88382_used[BR88382_AREA];     // centres already placed
static int br88382_validCentres[BR88382_AREA];       // centres whose whole disc fits the region
static int br88382_validCount;
static int br88382_budget;                           // search states left before giving up
static int br88382_bestDepth;                        // disc count of a cover that succeeded
static unsigned char br88382_comp[BR88382_AREA];     // full component, kept while its pieces are tried
static unsigned char br88382_assigned[BR88382_AREA]; // component cells already handed to a piece
static unsigned char br88382_cut[BR88382_AREA];      // one-tile-wide passage cells inside the component

static int br88382_idx(int x, int y) {
    return x * br88382_H + y;
}

static int br88382_region_count(void) {
    int n = 0;
    for (int i = 0; i < BR88382_AREA; i++) {
        n += br88382_region[i] != 0;
    }
    return n;
}

// Collect the 4-connected region of plain floor around `seed` into
// br88382_region, recording its cells in br88382_seen, and return its size.
static int br88382_collect_region(pos seed) {
    static int stack[BR88382_AREA];
    int sp = 0, count = 0;

    memset(br88382_region, 0, BR88382_AREA);
    stack[sp++] = br88382_idx(seed.x, seed.y);
    while (sp > 0) {
        const int i = stack[--sp];
        if (br88382_region[i]) {
            continue;
        }
        const pos p = posAt(i / br88382_H, i % br88382_H);
        if (terrainAt(p) != FLOOR) {
            continue;
        }
        br88382_region[i] = 1;
        br88382_seen[i] = 1;
        count++;
        if (p.x > 0)               stack[sp++] = i - br88382_H;
        if (p.x + 1 < br88382_W)   stack[sp++] = i + br88382_H;
        if (p.y > 0)               stack[sp++] = i - 1;
        if (p.y + 1 < br88382_H)   stack[sp++] = i + 1;
    }
    return count;
}

// List every centre in the region whose whole radius-2 disc also lies in it.
static void br88382_find_valid_centres(void) {
    br88382_validCount = 0;
    for (int x = 0; x < br88382_W; x++) {
        for (int y = 0; y < br88382_H; y++) {
            if (!br88382_region[br88382_idx(x, y)]) {
                continue;
            }
            boolean fits = true;
            for (int k = 0; k < BR88382_DISC_CELLS; k++) {
                const int nx = x + br88382_dx[k], ny = y + br88382_dy[k];
                if (nx < 0 || nx >= br88382_W || ny < 0 || ny >= br88382_H
                    || !br88382_region[br88382_idx(nx, ny)]) {
                    fits = false;
                    break;
                }
            }
            if (fits) {
                br88382_validCentres[br88382_validCount++] = br88382_idx(x, y);
            }
        }
    }
}

// Region cells that lie under no valid disc at all, so no disc placement can
// ever reach them; exactness means this must be zero.
static int br88382_unreachable_count(void) {
    int unreachable = 0;
    memset(br88382_covered, 0, BR88382_AREA);
    for (int v = 0; v < br88382_validCount; v++) {
        const int c = br88382_validCentres[v];
        const int cx = c / br88382_H, cy = c % br88382_H;
        for (int k = 0; k < BR88382_DISC_CELLS; k++) {
            br88382_covered[br88382_idx(cx + br88382_dx[k], cy + br88382_dy[k])] = 1;
        }
    }
    for (int i = 0; i < BR88382_AREA; i++) {
        if (br88382_region[i] && !br88382_covered[i]) {
            unreachable++;
        }
    }
    return unreachable;
}

// Cells of the region the discs have not reached.
static int br88382_uncovered_count(void) {
    int missed = 0;
    for (int i = 0; i < BR88382_AREA; i++) {
        if (br88382_region[i] && !br88382_covered[i]) {
            missed++;
        }
    }
    return missed;
}

// `depth` discs are already on the board.  Search for a disc that extends the
// union until the whole region is covered exactly, giving up at 9 discs or
// when the search budget runs out.
static boolean br88382_chain(int depth) {
    if (depth >= 3 && br88382_uncovered_count() == 0) {
        br88382_bestDepth = depth;
        return true;
    }
    if (depth >= BR88382_MAX_DISCS || --br88382_budget < 0) {
        return false;
    }
    for (int v = 0; v < br88382_validCount; v++) {
        const int c = br88382_validCentres[v];
        if (br88382_used[c] || !br88382_covered[c]) {
            continue; // each new centre must lie inside the union so far
        }
        int added[BR88382_DISC_CELLS], addedCount = 0;
        const int cx = c / br88382_H, cy = c % br88382_H;
        br88382_used[c] = 1;
        for (int k = 0; k < BR88382_DISC_CELLS; k++) {
            const int n = br88382_idx(cx + br88382_dx[k], cy + br88382_dy[k]);
            if (!br88382_covered[n]) {
                br88382_covered[n] = 1;
                added[addedCount++] = n;
            }
        }
        if (br88382_chain(depth + 1)) {
            return true;
        }
        for (int k = 0; k < addedCount; k++) {
            br88382_covered[added[k]] = 0;
        }
        br88382_used[c] = 0;
        if (br88382_budget < 0) {
            return false;
        }
    }
    return false;
}

// Whether the region is exactly one of the generator's plain circles: every
// cell inside dx*dx+dy*dy < r*r+r around some centre, for a circle radius the
// game can draw.  A circular room is itself a union of radius-2 discs (a
// radius-3 circle is exactly five of them), so without this check a plain
// circle would pass for a chunky room; a chunky room's discs meet in concave
// notches and can never be a plain circle.
static boolean br88382_is_single_circle(void) {
    static const int radiusSquares[10] = { 6, 12, 20, 30, 42, 56, 72, 90, 110, 132 };
    int minX = br88382_W, maxX = -1, minY = br88382_H, maxY = -1, size = 0;

    for (int x = 0; x < br88382_W; x++) {
        for (int y = 0; y < br88382_H; y++) {
            if (!br88382_region[br88382_idx(x, y)]) {
                continue;
            }
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
            size++;
        }
    }
    if (maxX - minX != maxY - minY) {
        return false; // a circle's bounding box is square
    }
    for (int cx = minX; cx <= maxX; cx++) {
        for (int cy = minY; cy <= maxY; cy++) {
            if (!br88382_region[br88382_idx(cx, cy)]) {
                continue; // the centre lies inside its own circle
            }
            for (int r = 0; r < 10; r++) {
                const int R = radiusSquares[r];
                int count = 0;
                boolean matches = true;
                for (int dx = -12; dx <= 12 && matches; dx++) {
                    for (int dy = -12; dy <= 12 && matches; dy++) {
                        if (dx * dx + dy * dy >= R) {
                            continue;
                        }
                        const int nx = cx + dx, ny = cy + dy;
                        if (nx < 0 || nx >= br88382_W || ny < 0 || ny >= br88382_H
                            || !br88382_region[br88382_idx(nx, ny)]) {
                            matches = false;
                            break;
                        }
                        count++;
                    }
                }
                if (matches && count == size) {
                    return true;
                }
            }
        }
    }
    return false;
}

// The region is built from at most two discs: one disc that equals it, or two
// chained discs whose union is it.  A chunky room always draws at least three
// circles, so such a region must be some simpler shape - but note a two-circle
// room can still admit a three-disc cover with different centres, which is why
// the smallest cover is checked directly here rather than by the search.
static boolean br88382_has_two_disc_cover(void) {
    const int size = br88382_region_count();

    if (size == BR88382_DISC_CELLS) {
        return true; // one disc exactly
    }
    if (size > 2 * BR88382_DISC_CELLS - 1) {
        return false; // two chained discs overlap, so their union is under 42 cells
    }
    for (int a = 0; a < br88382_validCount; a++) {
        const int ax = br88382_validCentres[a] / br88382_H;
        const int ay = br88382_validCentres[a] % br88382_H;
        for (int b = a + 1; b < br88382_validCount; b++) {
            const int bx = br88382_validCentres[b] / br88382_H;
            const int by = br88382_validCentres[b] % br88382_H;
            const int dx = bx - ax, dy = by - ay;
            if (dx * dx + dy * dy >= 6) {
                continue; // each centre must lie inside the other's disc
            }
            int overlap = 0;
            for (int k = 0; k < BR88382_DISC_CELLS; k++) {
                const int rx = ax + br88382_dx[k] - bx;
                const int ry = ay + br88382_dy[k] - by;
                if (rx * rx + ry * ry < 6) {
                    overlap++;
                }
            }
            if (2 * BR88382_DISC_CELLS - overlap == size) {
                return true; // both discs lie in the region, so their union is it
            }
        }
    }
    return false;
}

// Whether the region is exactly a chained union of 3..9 radius-2 discs; on
// success `discs` receives the number of discs used.
static boolean br88382_exact_chain_cover(int *discs) {
    br88382_find_valid_centres();
    if (br88382_validCount == 0 || br88382_unreachable_count() > 0) {
        return false;
    }
    if (br88382_has_two_disc_cover()) {
        return false;
    }
    br88382_budget = 50000; // shared by every seed, so one region costs at most this
    for (int s = 0; s < br88382_validCount; s++) {
        memset(br88382_covered, 0, BR88382_AREA);
        memset(br88382_used, 0, BR88382_AREA);
        if (br88382_budget < 0) {
            return false;
        }
        const int c = br88382_validCentres[s];
        const int cx = c / br88382_H, cy = c % br88382_H;
        br88382_used[c] = 1;
        for (int k = 0; k < BR88382_DISC_CELLS; k++) {
            br88382_covered[br88382_idx(cx + br88382_dx[k], cy + br88382_dy[k])] = 1;
        }
        if (br88382_chain(1)) {
            if (br88382_is_single_circle()) {
                return false; // a plain circular room, not a chunky one
            }
            *discs = br88382_bestDepth;
            return true;
        }
    }
    return false;
}

// Mark the one-tile-wide interior passage cells of the component: floor whose
// perpendicular neighbours are all outside the component while both axial
// neighbours are inside it.  Doorways, bolted hallways and the floor left by
// doorway sites that did not become doors are exactly such cells, so cutting
// them splits a component back into its rooms - even where loops give the
// rooms more than one connection.  A room itself never has such a cell: its
// discs overlap at least three tiles wide and its outer tips are dead ends.
static void br88382_mark_cuts(void) {
    for (int x = 0; x < br88382_W; x++) {
        for (int y = 0; y < br88382_H; y++) {
            const int i = br88382_idx(x, y);
            if (!br88382_comp[i]) {
                continue;
            }
            const boolean left = x > 0 && br88382_comp[br88382_idx(x - 1, y)];
            const boolean right = x + 1 < br88382_W && br88382_comp[br88382_idx(x + 1, y)];
            const boolean up = y > 0 && br88382_comp[br88382_idx(x, y - 1)];
            const boolean down = y + 1 < br88382_H && br88382_comp[br88382_idx(x, y + 1)];
            if ((left && right && !up && !down) || (up && down && !left && !right)) {
                br88382_cut[i] = 1;
            }
        }
    }
}

// Collect the next piece of the component (skipping its cut cells) into
// br88382_region and return its size, or 0 once every cell has been tried.
static int br88382_next_piece(void) {
    static int stack[BR88382_AREA];
    int start = -1;

    memset(br88382_region, 0, BR88382_AREA);
    for (int i = 0; i < BR88382_AREA; i++) {
        if (br88382_comp[i] && !br88382_cut[i] && !br88382_assigned[i]) {
            start = i;
            break;
        }
    }
    if (start < 0) {
        return 0;
    }
    int sp = 0, count = 0;
    stack[sp++] = start;
    br88382_region[start] = 1;
    br88382_assigned[start] = 1;
    while (sp > 0) {
        const int i = stack[--sp];
        count++;
        const int x = i / br88382_H, y = i % br88382_H;
        const int nx[4] = { x - 1, x + 1, x, x };
        const int ny[4] = { y, y, y - 1, y + 1 };
        for (int d = 0; d < 4; d++) {
            if (nx[d] < 0 || nx[d] >= br88382_W || ny[d] < 0 || ny[d] >= br88382_H) {
                continue;
            }
            const int n = br88382_idx(nx[d], ny[d]);
            if (br88382_comp[n] && !br88382_cut[n] && !br88382_region[n]) {
                br88382_region[n] = 1;
                br88382_assigned[n] = 1;
                stack[sp++] = n;
            }
        }
    }
    return count;
}

// Walk every floor region of depths 1-26 and count those that are exactly a
// chained union of radius-2 discs; the rule needs at least one to exist.
static boolean chunky_rooms_are_radius2_disc_chains(void) {
    int candidates = 0;

    br88382_W = levelWidth();
    br88382_H = levelHeight();
    memset(br88382_seen, 0, BR88382_AREA);

    for (int depth = 1; depth <= 26; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        for (int x = 0; x < br88382_W; x++) {
            for (int y = 0; y < br88382_H; y++) {
                const pos p = posAt(x, y);
                if (terrainAt(p) != FLOOR || br88382_seen[br88382_idx(x, y)]) {
                    continue;
                }
                br88382_collect_region(p);
                memcpy(br88382_comp, br88382_region, BR88382_AREA);
                int discs;
                if (br88382_exact_chain_cover(&discs)) {
                    candidates++;
                    continue;
                }
                // The component merges several rooms through door-sized
                // necks; cut those apart and try each room on its own.
                memset(br88382_cut, 0, BR88382_AREA);
                memset(br88382_assigned, 0, BR88382_AREA);
                br88382_mark_cuts();
                boolean anyCut = false;
                for (int i = 0; i < BR88382_AREA && !anyCut; i++) {
                    anyCut = br88382_cut[i] != 0;
                }
                if (!anyCut) {
                    continue;
                }
                int pieceSize;
                while ((pieceSize = br88382_next_piece()) > 0) {
                    if (pieceSize < 23) {
                        continue; // smaller than the smallest three-disc union
                    }
                    if (br88382_exact_chain_cover(&discs)) {
                        candidates++;
                    }
                }
            }
        }
    }

    TEST_ASSERT(candidates > 0,
                "some generated level should hold a room that is exactly a chained union of radius-2 discs");
    return !testFailed;
}

// ============================================================
// BR23784 — after the first room, up to 35 further rooms are
// placed, and no more than 35 placement attempts are made.
//
// On a finished level each placed room shows up as a region of
// dry floor: rooms are placed with a tile of clearance from all
// existing open space and connect to it only through one-tile
// doorway sites, which are door terrain when they become doors
// and plain floor when they do not — so a region never spans
// more than a few rooms merged through doorless doorways, and
// one room can never split across regions (its floor is one
// piece).  Hallways and loops hang off the rooms they open into
// and never form a region of their own.  The region count can
// therefore only undercount the rooms the generator placed, and
// "the first room plus at most 35 more" caps it at 36 on every
// level.  The controls require levels that really do hold
// several regions, so the bound cannot pass vacuously.  The
// attempt cap itself leaves no trace once the level exists and
// is not re-checked here.
// ============================================================

// Count the 4-connected regions of dry plain floor on the level.  Doors and
// other non-floor doorway outcomes break a region; liquid layered over the
// floor does not count as floor.
static int countDryFloorRegions(void) {
    int W = levelWidth(), H = levelHeight();
    static boolean seen[79][29];
    pos queue[79 * 29];
    int regions = 0;

    for (int x = 0; x < W; x++) {
        for (int y = 0; y < H; y++) {
            seen[x][y] = false;
        }
    }

    for (int sx = 0; sx < W; sx++) {
        for (int sy = 0; sy < H; sy++) {
            pos start = posAt(sx, sy);
            if (seen[sx][sy] || terrainAt(start) != FLOOR || cellHasLiquid(start)) {
                continue;
            }
            regions++;
            int head = 0, tail = 0;
            seen[sx][sy] = true;
            queue[tail++] = start;
            while (head < tail) {
                pos p = queue[head++];
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0 || (dx != 0 && dy != 0)) {
                            continue; // four-way connectivity
                        }
                        int nx = p.x + dx, ny = p.y + dy;
                        if (nx < 0 || nx >= W || ny < 0 || ny >= H || seen[nx][ny]) {
                            continue;
                        }
                        pos n = posAt(nx, ny);
                        if (terrainAt(n) == FLOOR && !cellHasLiquid(n)) {
                            seen[nx][ny] = true;
                            queue[tail++] = n;
                        }
                    }
                }
            }
        }
    }
    return regions;
}

// Generate depths 1 through 26 and check every level's floor-region count
// against the room cap, with a control that levels hold several regions.
static boolean br23784_room_count_capped(void) {
    int maxRegions = 0;
    for (int depth = 1; depth <= 26; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        int regions = countDryFloorRegions();
        if (regions > maxRegions) {
            maxRegions = regions;
        }
    }

    TEST_ASSERT(maxRegions <= 36,
                "the first room plus at most 35 more caps a level at 36 floor regions");
    TEST_ASSERT(maxRegions >= 2,
                "levels should hold several rooms, so the cap is not vacuous");
    return !testFailed;
}

// ============================================================
// BR70413 — the player counts as standing in darkness when the
// local light is more than 10 below the colour of the miner's
// light.
//
// The level is stripped to granite and the player pocketed in a
// five-tile floor cell, so the miner's own light is the only
// light anywhere.  Repainted from the player's tile, it fills
// that tile to the miner's light colour — nowhere near 10 below —
// and the player is not in darkness.  Six tiles away the local
// light is exactly zero: two rings of granite between it and the
// pocket stop every light ray, so every channel sits below the
// miner's colour minus 10 and a player standing there counts as
// standing in darkness.  Repainting from the new tile lifts the
// same tile straight back out of darkness, so what the rule's
// test follows is the local light against the threshold, not the
// location.
// ============================================================

static boolean br70413_darkness_follows_the_local_light(void) {
    clearMonsters();
    pos centre = posAt(levelWidth() / 2, levelHeight() / 2);

    // Strip the level to solid granite, then carve the player's pocket.
    for (int i = 0; i < levelWidth(); i++) {
        for (int j = 0; j < levelHeight(); j++) {
            setTerrain(posAt(i, j), GRANITE);
        }
    }
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(centre, dx, dy), FLOOR);
        }
    }
    setPlayerPos(centre);
    recomputeLighting();
    TEST_ASSERT(!playerIsInDarkness(),
                "a freshly painted miner's light fills the player's own tile, so it is not darkness");

    pos dark = posOffset(centre, 6, 0);
    setPlayerPos(dark);
    TEST_ASSERT_EQ(lightAt(dark), 0,
                   "control: light cannot cross the granite rings, so the tile's local light is zero");
    TEST_ASSERT(playerIsInDarkness(),
                "local light more than 10 below the miner's light counts as standing in darkness");

    recomputeLighting();
    TEST_ASSERT(!playerIsInDarkness(),
                "repainting the light from the new tile lifts it back out of darkness");
    return !testFailed;
}

// ============================================================

// ============================================================
// BR43163 — before carving, the whole level is filled with solid
// granite, and any liquid, gas or surface layer is cleared
// ============================================================

#define BR43163_FLOOR_MAX 4096
static pos br43163_oldFloor[BR43163_FLOOR_MAX];
static int br43163_oldFloorCount;

// Totals across the two level regenerations the test drives: the level's own
// single pmap only shows the generation being judged, so each regeneration is
// scored as it is in effect.
static int br43163_refilled;    // old floor now untouched granite
static int br43163_leftoverLiquid, br43163_leftoverGas, br43163_leftoverFire;

// Stamp every floor cell of the current level with a liquid, a surface and a
// gas layer.  The next level is generated over these exact cells -
// clearLevel() runs before the new carve - so the state they are found in
// afterwards is entirely the pre-carve wipe's doing: granite where the old
// level had floor proves the refill, and cleared layers prove the wipe.
static void br43163_stampLevel(void) {
    br43163_oldFloorCount = 0;
    for (int x = 0; x < levelWidth() && br43163_oldFloorCount < BR43163_FLOOR_MAX; x++) {
        for (int y = 0; y < levelHeight() && br43163_oldFloorCount < BR43163_FLOOR_MAX; y++) {
            pos p = posAt(x, y);
            if (terrainAt(p) != FLOOR || playerIsAt(p)) {
                continue;
            }
            br43163_oldFloor[br43163_oldFloorCount++] = p;
            spawnFeature(p, DF_SHALLOW_WATER);     // liquid layer
            spawnFeature(p, DF_PLAIN_FIRE);        // surface layer
            spawnFeature(p, DF_POISON_GAS_CLOUD);  // gas layer
        }
    }
    TEST_ASSERT(br43163_oldFloorCount > 0, "setup: the level should have floor to stamp");
    TEST_ASSERT(cellHasLiquid(br43163_oldFloor[0]), "setup: the liquid layer was stamped");
    TEST_ASSERT(cellIsOnFire(br43163_oldFloor[0]), "setup: the surface layer was stamped");
    TEST_ASSERT(cellHasGas(br43163_oldFloor[0]), "setup: the gas layer was stamped");
}

// Score the stamped cells as they now stand on the freshly generated level:
// cells the new carve left as untouched granite, judged for the layers the
// old level's stamps put on them.
static void br43163_judgeLevel(void) {
    for (int i = 0; i < br43163_oldFloorCount; i++) {
        pos p = br43163_oldFloor[i];
        if (terrainAt(p) != GRANITE) {
            continue;
        }
        br43163_refilled++;
        if (cellHasLiquid(p)) br43163_leftoverLiquid++;
        if (cellHasGas(p)) br43163_leftoverGas++;
        if (cellIsOnFire(p)) br43163_leftoverFire++;
    }
}

static void br43163_setup(void) {
    br43163_refilled = 0;
    br43163_leftoverLiquid = br43163_leftoverGas = br43163_leftoverFire = 0;

    br43163_stampLevel();
    descendToNextLevel();   // depth 2 is generated over the stamped depth 1
    br43163_judgeLevel();

    br43163_stampLevel();
    descendToNextLevel();   // depth 3 is generated over the stamped depth 2
    br43163_judgeLevel();
}

static boolean br43163_verify(void) {
    TEST_ASSERT(br43163_refilled > 0,
                "the new level should be filled with solid granite where the old level had floor, before carving");
    TEST_ASSERT_EQ(br43163_leftoverLiquid, 0,
                   "liquid from the previous level should be cleared before carving");
    TEST_ASSERT_EQ(br43163_leftoverGas, 0,
                   "gas from the previous level should be cleared before carving");
    TEST_ASSERT_EQ(br43163_leftoverFire, 0,
                   "surface effects from the previous level should be cleared before carving");
    return !testFailed;
}

// ============================================================
// BR25779 — A light source's own tile is cleared of shadow
// whenever the light dispels shadows, so the light's origin is
// never hidden in shadow.
//
// The level is stripped to bare floor first, so no glowing
// terrain, gas, liquid, or feature can clear a shadow anywhere;
// after the repaint the only lights left are a will-o'-the-wisp
// (whose light dispels shadows) and the player's miner's light
// (which maintains them).  The wisp takes the map corner
// farthest from the player — at least ~40 tiles away, while its
// light reaches at most 8 — so the two lights cannot overlap
// each other's tiles: the wisp's own origin tile must come out
// of the repaint unshadowed, the player's own origin tile must
// stay shadowed, and a tile beyond the wisp's reach must keep
// its shadow as the control.
// ============================================================

static boolean br25779_origin_cleared_and_controls_hold(void) {
    clearMonsters();

    // Strip the whole level to plain floor (clearing every layer), so the
    // only light sources that survive are the two placed here.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }

    const pos corners[4] = {
        posAt(1, 1),
        posAt(levelWidth() - 2, 1),
        posAt(1, levelHeight() - 2),
        posAt(levelWidth() - 2, levelHeight() - 2)
    };
    const pos playerPos = getPlayerPos();
    int farthest = 0;
    long farDist = -1;
    for (int i = 0; i < 4; i++) {
        const long dx = corners[i].x - playerPos.x;
        const long dy = corners[i].y - playerPos.y;
        if (dx * dx + dy * dy > farDist) {
            farDist = dx * dx + dy * dy;
            farthest = i;
        }
    }
    const pos source = corners[farthest];
    const pos control = corners[3 - farthest]; // the diagonally opposite corner

    TEST_ASSERT(placeHostileMonster(MK_WILL_O_THE_WISP, source) != NULL,
                "setup: the wisp should spawn in the chosen corner");
    recomputeLighting();

    TEST_ASSERT(lightAt(source) > 0,
                "control: the wisp's own tile holds light, so the wisp is a light source");
    TEST_ASSERT(!cellIsInShadow(source),
                "a light source that dispels shadows should clear its own origin tile of shadow");
    TEST_ASSERT(cellIsInShadow(control),
                "control: a tile beyond the light's reach keeps its shadow");
    TEST_ASSERT(cellIsInShadow(playerPos),
                "control: the miner's light maintains shadows, so your own light's origin tile stays in shadow");
    return !testFailed;
}

// ============================================================
// BR11884 — The chance that a newly placed room gets a
// hallway attached decreases with depth.
//
// Each room placement rolls the level profile's corridor
// chance, which the depth adjustment slides from 90% on depth 1
// down to 10% on the amulet level, and every hallway that ends
// up on the finished level is one such attachment (the scan
// counts only room-to-door hallway runs, as in BR26268).  With
// the same room counts offered at every depth, the total number
// of hallways over the shallow half of the dungeon must
// therefore exceed the total over the deep half.  Both totals
// also have to be positive: the chance falls with depth, it
// does not vanish, and an empty scan would prove nothing.
// ============================================================

static boolean br11884_hallways_fall_away_with_depth(void) {
    int shallow = 0, deep = 0;

    for (int depth = 1; depth <= 26; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        int hCount = 0, vCount = 0, maxH = 0, maxV = 0;
        scanHallways(&hCount, &vCount, &maxH, &maxV);
        if (depth <= 13) {
            shallow += hCount + vCount;
        } else {
            deep += hCount + vCount;
        }
    }

    TEST_ASSERT(shallow > 0,
                "control: shallow levels should carry hallways");
    TEST_ASSERT(deep > 0,
                "control: deep levels should still carry hallways (the chance falls, it does not vanish)");
    TEST_ASSERT(shallow > deep,
                "rooms get hallways attached less often at depth, so shallow levels should carry more hallways than deep ones");
    return !testFailed;
}

// ============================================================
// BR72185 — Cave rooms are blobs formed by smoothing random
// noise in one of three sizes: compact (3-12 wide by 4-8
// tall), tall north-south (3-12 wide by 15-27 tall), or wide
// east-west (20-27 wide by 4-8 tall).
//
// Rooms are recovered from the finished level with the same
// machinery the chunky-room test uses: 4-connected floor
// components are cut back into individual rooms at their
// one-tile doorway and hallway necks, so each piece's bounding
// box is one room's true footprint, hallway spurs and all.
// Solid rectangles (small rooms, vaults, straight passages),
// plain circles, and chained radius-2 disc unions (chunky
// rooms) are recognised and skipped — none of them is a cave —
// and among the remaining irregular pieces every footprint must
// fall in one of the three size bands, with all three bands
// actually observed across depths 1-26.  The tall band is the
// load-bearing one: nothing other than a tall cave (3-12 wide,
// 15-27 tall) can produce it — crosses top out at 8 rows,
// circles are square and would need to be 15+ wide, and chunky
// chains are excluded — so a generator that stops rolling tall
// caves fails the test.
// ============================================================

// Bounding box of the cells currently in br88382_region, as width and height.
static void br72185_region_bbox(int *outW, int *outH) {
    int minX = br88382_W, maxX = -1, minY = br88382_H, maxY = -1;
    for (int x = 0; x < br88382_W; x++) {
        for (int y = 0; y < br88382_H; y++) {
            if (br88382_region[br88382_idx(x, y)]) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }
    *outW = maxX - minX + 1;
    *outH = maxY - minY + 1;
}

// Cut one floor component into its individual rooms and fold each room's
// footprint into the three cave size-band counters.  A room that is a plain
// rectangle, a plain circle, or a chained union of radius-2 discs is not a
// cave and is skipped; an irregular room counts in a band when its bounding
// box lies in it.
static void br72185_count_rooms(int *rooms, int *compact, int *tall, int *wide) {
    memset(br88382_cut, 0, BR88382_AREA);
    memset(br88382_assigned, 0, BR88382_AREA);
    br88382_mark_cuts();

    int pieceSize;
    while ((pieceSize = br88382_next_piece()) > 0) {
        (void)pieceSize;
        (*rooms)++;
        int w, h;
        br72185_region_bbox(&w, &h);
        if (br88382_region_count() == w * h) {
            continue; // a solid rectangle: small room, vault, or straight passage
        }
        if (br88382_is_single_circle()) {
            continue; // a circular room
        }
        const boolean isTall = (w >= 3 && w <= 12 && h >= 15 && h <= 27);
        const boolean isWide = (w >= 20 && w <= 27 && h >= 4 && h <= 8);
        if (isTall || isWide) {
            int discs;
            if (br88382_exact_chain_cover(&discs)) {
                continue; // a chunky room: discs chained, not smoothed noise
            }
        }
        if (w >= 3 && w <= 12 && h >= 4 && h <= 8) {
            (*compact)++;
        } else if (isTall) {
            (*tall)++;
        } else if (isWide) {
            (*wide)++;
        }
    }
}

static boolean br72185_caves_fit_three_size_classes(void) {
    int rooms = 0, compact = 0, tall = 0, wide = 0;

    br88382_W = levelWidth();
    br88382_H = levelHeight();
    memset(br88382_seen, 0, BR88382_AREA);

    for (int depth = 1; depth <= 26; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        memset(br88382_seen, 0, BR88382_AREA);
        for (int x = 0; x < br88382_W; x++) {
            for (int y = 0; y < br88382_H; y++) {
                const pos p = posAt(x, y);
                if (terrainAt(p) != FLOOR || br88382_seen[br88382_idx(x, y)]) {
                    continue;
                }
                br88382_collect_region(p);
                memcpy(br88382_comp, br88382_region, BR88382_AREA);
                br72185_count_rooms(&rooms, &compact, &tall, &wide);
            }
        }
    }

    TEST_ASSERT(rooms > 0,
                "control: the generated levels should hold rooms at all");
    TEST_ASSERT(compact > 0,
                "some room should be a compact cave, 3-12 wide by 4-8 tall");
    TEST_ASSERT(tall > 0,
                "some room should be a tall north-south cave, 3-12 wide by 15-27 tall");
    TEST_ASSERT(wide > 0,
                "some room should be a wide east-west cave, 20-27 wide by 4-8 tall");
    return !testFailed;
}

// ============================================================
// BR32765 — a vault layout is chosen by weighted frequency from
// those whose depth range includes the current depth.
//
// At depth 1 exactly two reward-vault layouts are in range, one
// with twice the frequency of the other, while every deeper
// layout stands outside its range.  Sampling the engine's own
// raffle therefore has three observable consequences: an
// out-of-range layout is never drawn, each in-range layout is
// drawn sometimes, and the two in-range layouts split the draws
// in proportion to their frequencies rather than evenly.
// ============================================================

#define BR32765_MAX_LAYOUTS 64
#define BR32765_DEPTH 1
#define BR32765_TRIALS 3000

static boolean br32765_vault_layout_raffle_is_weighted_by_depth(void) {
    const int layoutCount = vaultLayoutCount();
    TEST_ASSERT(layoutCount > 1,
                "control: the game should define more than one reward vault layout");

    int counts[BR32765_MAX_LAYOUTS] = {0};
    int eligibleCount = 0, totalWeight = 0;

    for (int i = 0; i < layoutCount && i < BR32765_MAX_LAYOUTS; i++) {
        const int minDepth = vaultLayoutMinDepth(i);
        const int maxDepth = vaultLayoutMaxDepth(i);
        const int frequency = vaultLayoutFrequency(i);

        TEST_ASSERT(minDepth <= maxDepth,
                    "each vault layout should have a valid depth range");
        TEST_ASSERT(frequency > 0,
                    "each vault layout should enter the raffle with positive weight");

        if (minDepth <= BR32765_DEPTH && BR32765_DEPTH <= maxDepth) {
            eligibleCount++;
            totalWeight += frequency;
        }
    }
    TEST_ASSERT(eligibleCount >= 2,
                "control: depth 1 should offer at least two vault layouts");
    TEST_ASSERT(totalWeight > 0,
                "control: the in-range layouts should contribute raffle tickets");

    for (int trial = 0; trial < BR32765_TRIALS; trial++) {
        const int chosen = drawVaultLayout(BR32765_DEPTH);
        TEST_ASSERT(chosen >= 0 && chosen < layoutCount,
                    "the raffle should always choose one of the known vault layouts");
        if (chosen < 0 || chosen >= layoutCount) {
            break;
        }
        TEST_ASSERT(vaultLayoutMinDepth(chosen) <= BR32765_DEPTH
                        && BR32765_DEPTH <= vaultLayoutMaxDepth(chosen),
                    "a vault layout outside its depth range should never be chosen");
        counts[chosen]++;
    }

    for (int i = 0; i < layoutCount && i < BR32765_MAX_LAYOUTS; i++) {
        const boolean inRange = vaultLayoutMinDepth(i) <= BR32765_DEPTH
                                && BR32765_DEPTH <= vaultLayoutMaxDepth(i);
        if (!inRange) {
            TEST_ASSERT_EQ(counts[i], 0,
                           "a vault layout outside its depth range should not be chosen");
            continue;
        }

        const long expected = (long) BR32765_TRIALS * vaultLayoutFrequency(i) / totalWeight;
        const long tolerance = expected / 6 + 1;
        const long deviation = labs((long) counts[i] - expected);
        TEST_ASSERT(deviation <= tolerance,
                    "a vault layout should be chosen in proportion to its frequency");
        TEST_ASSERT(counts[i] > 0,
                    "every vault layout in range should be chosen sometimes");
    }
    return !testFailed;
}

// ============================================================
// BR39459 — the player is drawn with a distinct colour when
// invisible, in darkness, in shadow, or in light.
//
// A granite pocket washed only by the miner's light leaves the
// player in shadow, because the miner's light maintains shadow.
// A fungus on the player's tile lights it and dispels that
// shadow; invisibility overrides the lighting entirely.  The
// fourth state takes the local light from the map: moving the
// player six tiles into the granite leaves a tile whose local
// light is zero, which counts as darkness when the colour is
// recomputed.  The four chosen colours must all differ.
// ============================================================

#define BR39459_POCKET_RADIUS 2

static pos br39459_centre;

static void br39459_setup(void) {
    removeStatusFromPlayer(STATUS_INVISIBLE);
    setLightMultiplier(1);
    setPlayerInWater(false);
    clearMonsters();

    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }
    br39459_centre = posAt(levelWidth() / 2, levelHeight() / 2);
    for (int dx = -BR39459_POCKET_RADIUS; dx <= BR39459_POCKET_RADIUS; dx++) {
        for (int dy = -BR39459_POCKET_RADIUS; dy <= BR39459_POCKET_RADIUS; dy++) {
            setTerrain(posOffset(br39459_centre, dx, dy), FLOOR);
        }
    }
    setPlayerPos(br39459_centre);
    recomputeLighting();
}

static boolean br39459_player_has_distinct_lighting_colors(void) {
    TEST_ASSERT(strcmp(playerDrawColorName(), "shadow") == 0,
                "a player lit only by the miner's light should be drawn in shadow");
    const unsigned long shadowColor = playerDrawColorId();

    spawnFeature(br39459_centre, DF_LUMINESCENT_FUNGUS);
    recomputeLighting();
    TEST_ASSERT(strcmp(playerDrawColorName(), "light") == 0,
                "a player standing in light should be drawn in the light colour");
    const unsigned long lightColor = playerDrawColorId();

    applyStatusToPlayer(STATUS_INVISIBLE, 100);
    recomputeLighting();
    TEST_ASSERT(strcmp(playerDrawColorName(), "invisible") == 0,
                "an invisible player should be drawn in the invisibility colour");
    const unsigned long invisibleColor = playerDrawColorId();

    removeStatusFromPlayer(STATUS_INVISIBLE);
    const pos darkTile = posOffset(br39459_centre, 6, 0);
    setPlayerPos(darkTile);
    updatePlayerDrawColor();
    TEST_ASSERT(playerIsInDarkness(),
                "control: the granite tile should have no local light");
    TEST_ASSERT(strcmp(playerDrawColorName(), "darkness") == 0,
                "a player in darkness should be drawn in the darkness colour");
    const unsigned long darknessColor = playerDrawColorId();

    TEST_ASSERT(shadowColor != lightColor,
                "shadow and light should be distinct colours");
    TEST_ASSERT(shadowColor != invisibleColor,
                "shadow and invisibility should be distinct colours");
    TEST_ASSERT(shadowColor != darknessColor,
                "shadow and darkness should be distinct colours");
    TEST_ASSERT(lightColor != invisibleColor,
                "light and invisibility should be distinct colours");
    TEST_ASSERT(lightColor != darknessColor,
                "light and darkness should be distinct colours");
    TEST_ASSERT(invisibleColor != darknessColor,
                "invisibility and darkness should be distinct colours");
    return !testFailed;
}

// ============================================================
// BR94157 — On depth 1 the first room is always the large
// entrance room, while on deeper levels the first room may
// instead be a cavern that can span the whole level.
//
// The framework records the first room the generator carves on
// each fresh level, before any other room is attached, so the
// test can examine that room alone rather than the merged floor
// a finished level presents.
//
// On depth 1 the first room must be exactly the entrance room:
// the 8x10 rectangle joined to the 20x5 rectangle below it,
// forming the upside-down T at the bottom of the map.  Every
// tile of that T must belong to the first room, and the room
// holds 140 tiles in all, so any other room shape also fails
// the count.
//
// On the deeper levels the first room must sometimes be the
// level-filling cavern: its footprint must reach the cavern's
// minimum 50 tiles wide by 20 tall, which no other room shape
// can reach (caves top out at 27 tiles across).  A control
// requires at least one deeper first room to stay below that
// bound, so the rule's "may instead" is not satisfied by
// making every deeper level's first room a cavern.
// ============================================================

#define BR94157_CAVERN_MIN_WIDTH  50
#define BR94157_CAVERN_MIN_HEIGHT 20
#define BR94157_LAST_DEPTH        26

static int br94157_first_room_tile_count(void) {
    int count = 0;
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            if (firstRoomCovers(posAt(x, y))) {
                count++;
            }
        }
    }
    return count;
}

static boolean br94157_first_room_spans_level(void) {
    int minX = levelWidth(), maxX = -1, minY = levelHeight(), maxY = -1;
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            if (!firstRoomCovers(posAt(x, y))) {
                continue;
            }
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }
    return maxX >= 0
            && maxX - minX + 1 >= BR94157_CAVERN_MIN_WIDTH
            && maxY - minY + 1 >= BR94157_CAVERN_MIN_HEIGHT;
}

static boolean br94157_first_room_entrance_on_depth_1_cavern_below(void) {
    const int W = levelWidth(), H = levelHeight();
    // The entrance room: an 8x10 rectangle joined to a 20x5 rectangle below it.
    const int narrowX = W / 2 - 8 / 2 - 1, narrowY = H - 10 - 2;
    const int wideX = W / 2 - 20 / 2 - 1, wideY = H - 5 - 2;

    for (int x = narrowX; x < narrowX + 8; x++) {
        for (int y = narrowY; y < narrowY + 10; y++) {
            TEST_ASSERT(firstRoomCovers(posAt(x, y)),
                        "the 8x10 part of the entrance room should be first-room floor on depth 1");
        }
    }
    for (int x = wideX; x < wideX + 20; x++) {
        for (int y = wideY; y < wideY + 5; y++) {
            TEST_ASSERT(firstRoomCovers(posAt(x, y)),
                        "the 20x5 part of the entrance room should be first-room floor on depth 1");
        }
    }
    TEST_ASSERT_EQ(br94157_first_room_tile_count(), 8 * 10 + 20 * 5 - 8 * 5,
                   "the depth-1 first room should be exactly the entrance room");
    TEST_ASSERT(!br94157_first_room_spans_level(),
                "the entrance room should not reach the level-filling cavern's size");

    boolean sawCavern = false, sawOther = false;
    for (int depth = 2; depth <= BR94157_LAST_DEPTH; depth++) {
        while (currentDepth() < depth) {
            descendToNextLevel();
        }
        if (br94157_first_room_spans_level()) {
            sawCavern = true;
        } else {
            sawOther = true;
        }
    }

    TEST_ASSERT(sawCavern,
                "some deeper level's first room should be a level-filling cavern at least 50 by 20 tiles");
    TEST_ASSERT(sawOther,
                "control: the first room should not be a level-filling cavern on every deeper level");
    return !testFailed;
}

// ============================================================
// BR49485 — a room-like vault grows outward from a doorway
// whose blocked-off region size falls within the vault's
// required room-size range; doorway candidates are rated by
// how many tiles would be cut off if the doorway were blocked.
//
// The fixture replaces the level with one dead-end chamber
// behind a single doorway, then asks the game to build a
// room-like vault.  The chamber's floor is exactly the region
// that would be cut off by blocking its doorway.
// ============================================================

static boolean br49485_built;
static int br49485_roomCells;

static int countVaultInteriorCells(void);

static void br49485_in_range_setup(void) {
    br49485_roomCells = roomVaultRoomSizeMin();
    br49485_built = buildRoomVaultAtChamber(br49485_roomCells);
}

static void br49485_out_of_range_setup(void) {
    br49485_roomCells = roomVaultRoomSizeMin() - 1;
    if (br49485_roomCells < 4) {
        br49485_roomCells = roomVaultRoomSizeMax() + 1;
    }
    br49485_built = buildRoomVaultAtChamber(br49485_roomCells);
}

static boolean br49485_in_range_vault_grows_from_doorway(void) {
    TEST_ASSERT(br49485_built,
                "a doorway whose blocked-off region is inside the room-size range should start a room vault");
    TEST_ASSERT_EQ(chamberFloorCellCount(), br49485_roomCells,
                   "control: the chamber should be as large as requested");
    TEST_ASSERT(br49485_roomCells >= roomVaultRoomSizeMin()
                    && br49485_roomCells <= roomVaultRoomSizeMax(),
                "control: the chamber should lie inside the vault's room-size range");

    int covered = 0;
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos loc = posAt(x, y);
            if (!cellIsVaultInterior(loc)) {
                continue;
            }
            covered++;
            TEST_ASSERT(chamberFloorContains(loc) || chamberDoorwayAt(loc),
                        "the vault should cover only its doorway and the blocked-off region behind it");
        }
    }
    TEST_ASSERT_EQ(covered, br49485_roomCells + 1,
                   "the vault should grow from the doorway across exactly the tiles that would be cut off, plus the doorway");

    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos loc = posAt(x, y);
            if (chamberFloorContains(loc)) {
                TEST_ASSERT(cellIsVaultInterior(loc),
                            "every floor tile of the blocked-off region should become vault interior");
            }
        }
    }
    return !testFailed;
}

static boolean br49485_out_of_range_doorway_is_rejected(void) {
    TEST_ASSERT(!br49485_built,
                "a doorway whose blocked-off region is outside the room-size range should not start a room vault");
    TEST_ASSERT_EQ(chamberFloorCellCount(), br49485_roomCells,
                   "control: the chamber should be as large as requested");
    TEST_ASSERT(!(br49485_roomCells >= roomVaultRoomSizeMin()
                  && br49485_roomCells <= roomVaultRoomSizeMax()),
                "control: the chamber should lie outside the vault's room-size range");
    TEST_ASSERT_EQ(countVaultInteriorCells(), 0,
                   "a rejected doorway should leave no vault interior behind");
    return !testFailed;
}

// ============================================================
// BR22946 — a vault adopts an item generated elsewhere, hands
// an item to a sub-vault, or builds a guarded vestibule in a
// doorway; if the sub-vault cannot be built after ten tries,
// the whole vault aborts.
//
// The fixture builds the vault whose doorway is guarded by a
// locked-door vestibule.  The vestibule generates its door key
// and hands it to an adopting child vault, so a built vault
// leaves a locked doorway in its interior and its key inside a
// vault rather than on the door.  When child vaults cannot be
// built, the same fixture must abort the whole vault: no
// doorway, key, item, creature, or interior tile may remain.
// ============================================================

static int countDoorKeys(void) {
    int keys = 0;
    for (int i = 0; i < countFloorItems(); i++) {
        item *theItem = floorItemAtIndex(i);
        if (itemCategory(theItem) == KEY && itemKind(theItem) == KEY_DOOR) {
            keys++;
        }
    }
    return keys;
}

static int countVaultInteriorCells(void) {
    int cells = 0;
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            if (cellIsVaultInterior(posAt(x, y))) {
                cells++;
            }
        }
    }
    return cells;
}

static boolean br22946_vaultBuilt;
static int br22946_lockedDoorsBefore;
static int br22946_keysBefore;
static int br22946_itemsBefore;
static int br22946_monstersBefore;
static int br22946_vaultCellsBefore;

static void br22946_build_setup(void) {
    br22946_lockedDoorsBefore = countTerrain(LOCKED_DOOR);
    br22946_keysBefore = countDoorKeys();
    br22946_itemsBefore = countFloorItems();
    br22946_monstersBefore = monsterCount();
    br22946_vaultCellsBefore = countVaultInteriorCells();
    br22946_vaultBuilt = buildGuardedDoorVault(false);
}

static void br22946_failed_subvault_setup(void) {
    br22946_lockedDoorsBefore = countTerrain(LOCKED_DOOR);
    br22946_keysBefore = countDoorKeys();
    br22946_itemsBefore = countFloorItems();
    br22946_monstersBefore = monsterCount();
    br22946_vaultCellsBefore = countVaultInteriorCells();
    br22946_vaultBuilt = buildGuardedDoorVault(true);
}

static boolean br22946_vault_adopts_key_through_guarded_door(void) {
    TEST_ASSERT(br22946_vaultBuilt,
                "the vault should be built when its child vault can be built");
    TEST_ASSERT(countTerrain(LOCKED_DOOR) > br22946_lockedDoorsBefore,
                "the vault should guard its doorway with a locked door");
    TEST_ASSERT(countDoorKeys() > br22946_keysBefore,
                "the guarded doorway should generate a door key");
    TEST_ASSERT(countVaultInteriorCells() > br22946_vaultCellsBefore,
                "the built vault should claim interior tiles");

    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            if (cellIsTerrain(posAt(x, y), LOCKED_DOOR)) {
                TEST_ASSERT(cellIsVaultInterior(posAt(x, y)),
                            "a guarded doorway should belong to a vault");
            }
        }
    }
    for (int i = 0; i < countFloorItems(); i++) {
        item *theItem = floorItemAtIndex(i);
        if (itemCategory(theItem) != KEY || itemKind(theItem) != KEY_DOOR) {
            continue;
        }
        TEST_ASSERT(!cellIsTerrain(itemPosition(theItem), LOCKED_DOOR),
                    "a generated key should be handed off, not left in its doorway");
        TEST_ASSERT(cellIsVaultInterior(itemPosition(theItem)),
                    "a generated key should be adopted into a vault");
    }
    return !testFailed;
}

static boolean br22946_failed_subvault_aborts_whole_vault(void) {
    TEST_ASSERT(!br22946_vaultBuilt,
                "a vault whose child vault cannot be built should abort");
    TEST_ASSERT_EQ(countTerrain(LOCKED_DOOR), br22946_lockedDoorsBefore,
                   "an aborted vault should leave no doorway behind");
    TEST_ASSERT_EQ(countDoorKeys(), br22946_keysBefore,
                   "an aborted vault should leave no key behind");
    TEST_ASSERT_EQ(countFloorItems(), br22946_itemsBefore,
                   "an aborted vault should leave none of its items behind");
    TEST_ASSERT_EQ(monsterCount(), br22946_monstersBefore,
                   "an aborted vault should leave no creatures behind");
    TEST_ASSERT_EQ(countVaultInteriorCells(), br22946_vaultCellsBefore,
                   "an aborted vault should claim no interior tiles");
    return !testFailed;
}

// ============================================================
// BR36818 — an open-area vault expands from a random floor tile
// along path distance until it reaches a randomly chosen size,
// and is rejected if it would engulf an item, a monster, or
// another vault.
//
// Every scenario first strips the level to solid wall and carves
// a single known floor region, so no generated terrain, item or
// monster can affect the outcome.  The growth test repeats the
// build on the same open room and checks, for each build, that
// the interior is exactly a prefix of the four-way pathing
// distances from the recorded origin: every tile closer to the
// origin than the farthest interior tile is inside, the origin
// itself is inside, and the size lies within the vault's
// advertised range.  Across builds both the origin and the size
// must vary, showing that both are chosen at random.  The
// rejection tests compare the same corridor with and without the
// obstruction; the control must build, and the obstructed build
// must fail without claiming tiles or disturbing the obstruction.
// ============================================================

#define BR36818_TRIALS 8

static boolean br36818_built[BR36818_TRIALS];
static int br36818_size[BR36818_TRIALS];
static boolean br36818_prefixHolds[BR36818_TRIALS];
static boolean br36818_originOnFloor[BR36818_TRIALS];
static boolean br36818_originInside[BR36818_TRIALS];
static pos br36818_origin[BR36818_TRIALS];

static void br36818_stripLevel(void) {
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), WALL);
        }
    }
}

static void br36818_carveRect(int x0, int y0, int x1, int y1) {
    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
}

// Whether the current vault interior is exactly the set of tiles within some
// four-way path distance of the recorded origin.  The engine processes the
// distance rings in order, so it must include every tile closer than the
// farthest interior tile and may include only part of that farthest ring.
static boolean br36818_interiorIsDistancePrefix(void) {
    int maxDistance = -1;

    if (!isPosInMap(openAreaVaultOrigin())
        || !cellIsVaultInterior(openAreaVaultOrigin())) {
        return false;
    }

    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos loc = posAt(x, y);
            if (!cellIsVaultInterior(loc)) {
                continue;
            }
            int distance = openAreaVaultPathDistance(loc);
            if (distance < 0) {
                return false; // an interior tile the origin cannot path to
            }
            if (distance > maxDistance) {
                maxDistance = distance;
            }
        }
    }
    if (maxDistance < 0) {
        return false;
    }

    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos loc = posAt(x, y);
            int distance = openAreaVaultPathDistance(loc);
            if (distance >= 0 && distance < maxDistance && !cellIsVaultInterior(loc)) {
                return false; // a closer tile was skipped while a farther one was taken
            }
        }
    }

    // The growth stops partway through the farthest ring, so at least one tile
    // at that distance must be inside.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            pos loc = posAt(x, y);
            if (cellIsVaultInterior(loc)
                && openAreaVaultPathDistance(loc) == maxDistance) {
                return true;
            }
        }
    }
    return false;
}

static void br36818_growth_setup(void) {
    clearMonsters();
    for (int trial = 0; trial < BR36818_TRIALS; trial++) {
        br36818_stripLevel();
        br36818_carveRect(20, 7, 58, 21); // a wide open room
        setPlayerPos(posAt(39, 14));

        br36818_built[trial] = buildOpenAreaVault();
        br36818_origin[trial] = openAreaVaultOrigin();
        br36818_size[trial] = countVaultInteriorCells();
        br36818_originOnFloor[trial] = (terrainAt(br36818_origin[trial]) == FLOOR);
        br36818_originInside[trial] = cellIsVaultInterior(br36818_origin[trial]);
        br36818_prefixHolds[trial] = br36818_interiorIsDistancePrefix();
    }
}

static boolean br36818_grows_from_random_floor_tile_to_random_size(void) {
    const int minSize = openAreaVaultTargetMin();
    const int maxSize = openAreaVaultTargetMax();
    int distinctOrigins = 0;
    int distinctSizes = 0;

    TEST_ASSERT(minSize > 0 && maxSize >= minSize,
                "control: the vault should advertise a target-size range");

    for (int trial = 0; trial < BR36818_TRIALS; trial++) {
        TEST_ASSERT(br36818_built[trial],
                    "an unobstructed open room should let the vault be built");
        TEST_ASSERT(br36818_originOnFloor[trial],
                    "the vault should grow from a floor tile");
        TEST_ASSERT(br36818_originInside[trial],
                    "the origin tile should belong to the vault interior");
        TEST_ASSERT(br36818_size[trial] >= minSize
                        && br36818_size[trial] <= maxSize,
                    "the vault should grow to a size inside its target range");
        TEST_ASSERT(br36818_prefixHolds[trial],
                    "the vault should take tiles in path-distance order from its origin");

        boolean newOrigin = true;
        boolean newSize = true;
        for (int earlier = 0; earlier < trial; earlier++) {
            if (posAt(br36818_origin[earlier].x, br36818_origin[earlier].y).x
                    == br36818_origin[trial].x
                && br36818_origin[earlier].y == br36818_origin[trial].y) {
                newOrigin = false;
            }
            if (br36818_size[earlier] == br36818_size[trial]) {
                newSize = false;
            }
        }
        distinctOrigins += newOrigin ? 1 : 0;
        distinctSizes += newSize ? 1 : 0;
    }

    TEST_ASSERT(distinctOrigins > 1,
                "the vault should pick its starting floor tile at random");
    TEST_ASSERT(distinctSizes > 1,
                "the vault should pick its target size at random");
    return !testFailed;
}

#define BR36818_CORRIDOR_X0 10
#define BR36818_CORRIDOR_X1 50
#define BR36818_CORRIDOR_Y 14
#define BR36818_ORIGIN_X 10
#define BR36818_OBSTRUCTION_X 20
#define BR36818_TARGET 20

static boolean br36818_itemControlBuilt;
static boolean br36818_itemBlockedBuilt;
static int br36818_itemControlSize;
static int br36818_itemBlockedSize;
static pos br36818_itemLocation;

static void br36818_item_setup(void) {
    clearMonsters();

    br36818_stripLevel();
    br36818_carveRect(BR36818_CORRIDOR_X0, BR36818_CORRIDOR_Y,
                      BR36818_CORRIDOR_X1, BR36818_CORRIDOR_Y);
    setPlayerPos(posAt(35, BR36818_CORRIDOR_Y));
    br36818_itemControlBuilt =
        buildOpenAreaVaultAt(posAt(BR36818_ORIGIN_X, BR36818_CORRIDOR_Y), BR36818_TARGET);
    br36818_itemControlSize = countVaultInteriorCells();

    br36818_stripLevel();
    br36818_carveRect(BR36818_CORRIDOR_X0, BR36818_CORRIDOR_Y,
                      BR36818_CORRIDOR_X1, BR36818_CORRIDOR_Y);
    setPlayerPos(posAt(35, BR36818_CORRIDOR_Y));
    br36818_itemLocation = posAt(BR36818_OBSTRUCTION_X, BR36818_CORRIDOR_Y);
    placeItemOnFloor(FOOD, RATION, br36818_itemLocation);
    br36818_itemBlockedBuilt =
        buildOpenAreaVaultAt(posAt(BR36818_ORIGIN_X, BR36818_CORRIDOR_Y), BR36818_TARGET);
    br36818_itemBlockedSize = countVaultInteriorCells();
}

static boolean br36818_rejects_engulfing_an_item(void) {
    TEST_ASSERT(br36818_itemControlBuilt,
                "control: a clear corridor should let the vault be built");
    TEST_ASSERT_EQ(br36818_itemControlSize, BR36818_TARGET,
                   "control: the clear vault should reach its target size");

    TEST_ASSERT(!br36818_itemBlockedBuilt,
                "a vault that would engulf an item should be rejected");
    TEST_ASSERT_EQ(br36818_itemBlockedSize, 0,
                   "a rejected vault should claim no interior tiles");

    item *theItem = findFloorItem(FOOD, RATION);
    TEST_ASSERT(theItem != NULL,
                "a rejected vault should leave the item where it was");
    TEST_ASSERT_POS_EQ(itemPosition(theItem), br36818_itemLocation,
                       "the item should not be moved or removed");
    return !testFailed;
}

static boolean br36818_monsterControlBuilt;
static boolean br36818_monsterBlockedBuilt;
static int br36818_monsterControlSize;
static int br36818_monsterBlockedSize;
static creature *br36818_obstructingMonster;
static pos br36818_monsterLocation;

static void br36818_monster_setup(void) {
    clearMonsters();

    br36818_stripLevel();
    br36818_carveRect(BR36818_CORRIDOR_X0, BR36818_CORRIDOR_Y,
                      BR36818_CORRIDOR_X1, BR36818_CORRIDOR_Y);
    setPlayerPos(posAt(35, BR36818_CORRIDOR_Y));
    br36818_monsterControlBuilt =
        buildOpenAreaVaultAt(posAt(BR36818_ORIGIN_X, BR36818_CORRIDOR_Y), BR36818_TARGET);
    br36818_monsterControlSize = countVaultInteriorCells();

    br36818_stripLevel();
    br36818_carveRect(BR36818_CORRIDOR_X0, BR36818_CORRIDOR_Y,
                      BR36818_CORRIDOR_X1, BR36818_CORRIDOR_Y);
    setPlayerPos(posAt(35, BR36818_CORRIDOR_Y));
    br36818_monsterLocation = posAt(BR36818_OBSTRUCTION_X, BR36818_CORRIDOR_Y);
    br36818_obstructingMonster = spawnMonster(MK_RAT, br36818_monsterLocation);
    br36818_monsterBlockedBuilt =
        buildOpenAreaVaultAt(posAt(BR36818_ORIGIN_X, BR36818_CORRIDOR_Y), BR36818_TARGET);
    br36818_monsterBlockedSize = countVaultInteriorCells();
}

static boolean br36818_rejects_engulfing_a_monster(void) {
    TEST_ASSERT(br36818_monsterControlBuilt,
                "control: a clear corridor should let the vault be built");
    TEST_ASSERT_EQ(br36818_monsterControlSize, BR36818_TARGET,
                   "control: the clear vault should reach its target size");

    TEST_ASSERT(!br36818_monsterBlockedBuilt,
                "a vault that would engulf a monster should be rejected");
    TEST_ASSERT_EQ(br36818_monsterBlockedSize, 0,
                   "a rejected vault should claim no interior tiles");
    TEST_ASSERT(br36818_obstructingMonster != NULL
                    && monsterIsAlive(br36818_obstructingMonster),
                "a rejected vault should leave the monster alive");
    if (br36818_obstructingMonster != NULL) {
        TEST_ASSERT_POS_EQ(monsterPosition(br36818_obstructingMonster),
                           br36818_monsterLocation,
                           "the monster should not be moved or removed");
    }
    return !testFailed;
}

// The second vault starts two tiles from the corridor's right end, so its
// growth has almost nowhere to go but left, toward the first vault.  Its
// target size is exactly the number of tiles it takes to reach the first
// vault's near edge; one more tile would engulf it.
#define BR36818_VAULT_CORRIDOR_X1 50
#define BR36818_FIRST_ORIGIN_X 10
#define BR36818_FIRST_TARGET 10
#define BR36818_SECOND_ORIGIN_X 48
#define BR36818_SECOND_TARGET 32

static boolean br36818_vaultControlBuilt;
static int br36818_vaultControlSize;
static boolean br36818_firstVaultBuilt;
static boolean br36818_secondVaultBuilt;
static int br36818_firstVaultSize;
static int br36818_tilesAfterFirst;
static int br36818_tilesAfterSecond;

static void br36818_vault_setup(void) {
    clearMonsters();

    // Control: the second vault's growth alone, on a clear corridor.
    br36818_stripLevel();
    br36818_carveRect(BR36818_CORRIDOR_X0, BR36818_CORRIDOR_Y,
                      BR36818_VAULT_CORRIDOR_X1, BR36818_CORRIDOR_Y);
    setPlayerPos(posAt(45, BR36818_CORRIDOR_Y));
    br36818_vaultControlBuilt =
        buildOpenAreaVaultAt(posAt(BR36818_SECOND_ORIGIN_X, BR36818_CORRIDOR_Y),
                             BR36818_SECOND_TARGET);
    br36818_vaultControlSize = countVaultInteriorCells();

    // A first vault near the left end, then a second whose growth toward it
    // must run into the first vault's tiles before reaching its target.
    br36818_stripLevel();
    br36818_carveRect(BR36818_CORRIDOR_X0, BR36818_CORRIDOR_Y,
                      BR36818_VAULT_CORRIDOR_X1, BR36818_CORRIDOR_Y);
    setPlayerPos(posAt(45, BR36818_CORRIDOR_Y));
    br36818_firstVaultBuilt =
        buildOpenAreaVaultAt(posAt(BR36818_FIRST_ORIGIN_X, BR36818_CORRIDOR_Y),
                             BR36818_FIRST_TARGET);
    br36818_firstVaultSize = countVaultInteriorCells();
    br36818_tilesAfterFirst = br36818_firstVaultSize;

    br36818_secondVaultBuilt =
        buildOpenAreaVaultAt(posAt(BR36818_SECOND_ORIGIN_X, BR36818_CORRIDOR_Y),
                             BR36818_SECOND_TARGET);
    br36818_tilesAfterSecond = countVaultInteriorCells();
}

static boolean br36818_rejects_engulfing_another_vault(void) {
    TEST_ASSERT(br36818_vaultControlBuilt,
                "control: the same corridor without an earlier vault should accept the second vault");
    TEST_ASSERT_EQ(br36818_vaultControlSize, BR36818_SECOND_TARGET,
                   "control: the second vault should reach its target size");

    TEST_ASSERT(br36818_firstVaultBuilt,
                "the first vault should build on the clear corridor");
    TEST_ASSERT_EQ(br36818_firstVaultSize, BR36818_FIRST_TARGET,
                   "the first vault should reach its target size");

    TEST_ASSERT(!br36818_secondVaultBuilt,
                "a vault whose growth would engulf another vault should be rejected");
    TEST_ASSERT_EQ(br36818_tilesAfterSecond, br36818_tilesAfterFirst,
                   "the rejected vault should add no interior tiles");
    return !testFailed;
}

const testCase tests_dungeon[] = {
    { "BR94157_first_room_entrance_on_depth_1_cavern_below", NULL, NULL, br94157_first_room_entrance_on_depth_1_cavern_below },
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
    { "BR33256_pathfinding_avoids_damage_immune_stationary_monsters", NULL, NULL, damage_immune_stationary_monsters_are_pathfinding_obstacles },
    { "BR68830_dropped_item_lands_on_nearest_reachable_tile", br68830_drop_setup, br68830_drop_step, dropped_item_lands_on_nearest_reachable_tile },

    { "BR79876_gas_feature_adds_start_probability_to_volume", gas_volume_setup, gas_dissipates_step, stacked_gas_volume_outlasts_single },

    { "BR26376_vaults_purge_liquids_from_interiors", NULL, NULL, vaults_purge_liquids_from_their_interiors },

    { "BR26268_hallways_respect_length_bounds", NULL, NULL, hallways_respect_length_bounds },

    { "BR93781_bridges_meet_placement_conditions", NULL, NULL, bridges_meet_placement_conditions },

    { "BR44816_generated_levels_stay_connected",  NULL, NULL, br44816_levels_stay_connected },

    { "BR24979_lake_bodies_fit_the_generation_canvas", NULL, NULL, lake_bodies_fit_the_generation_canvas },

    { "BR88382_chunky_rooms_are_radius2_disc_chains", NULL, NULL, chunky_rooms_are_radius2_disc_chains },

    { "BR23784_room_count_capped_after_the_first", NULL, NULL, br23784_room_count_capped },
    { "BR70413_darkness_when_local_light_below_miners", NULL, NULL, br70413_darkness_follows_the_local_light },

    { "BR43163_level_refilled_with_granite_and_cleared_layers", br43163_setup, NULL, br43163_verify },

    { "BR25779_light_origin_cleared_of_shadow",   NULL, NULL, br25779_origin_cleared_and_controls_hold },
    { "BR11884_hallways_fall_away_with_depth",    NULL, NULL, br11884_hallways_fall_away_with_depth },
    { "BR72185_caves_fit_three_size_classes",      NULL, NULL, br72185_caves_fit_three_size_classes },

    { "BR22946_vault_adopts_key_through_guarded_door", br22946_build_setup, NULL, br22946_vault_adopts_key_through_guarded_door },
    { "BR22946_failed_subvault_aborts_whole_vault",    br22946_failed_subvault_setup, NULL, br22946_failed_subvault_aborts_whole_vault },

    { "BR32765_vault_layout_raffle_weighted_by_depth", NULL, NULL, br32765_vault_layout_raffle_is_weighted_by_depth },
    { "BR49485_room_vault_grows_from_in_range_doorway", br49485_in_range_setup, NULL, br49485_in_range_vault_grows_from_doorway },
    { "BR49485_room_vault_rejects_out_of_range_doorway", br49485_out_of_range_setup, NULL, br49485_out_of_range_doorway_is_rejected },
    { "BR39459_player_lighting_colors_are_distinct",   br39459_setup, NULL, br39459_player_has_distinct_lighting_colors },

    { "BR36818_open_area_vault_grows_from_random_tile_to_random_size",
      br36818_growth_setup, NULL, br36818_grows_from_random_floor_tile_to_random_size },
    { "BR36818_open_area_vault_rejects_engulfing_item",
      br36818_item_setup, NULL, br36818_rejects_engulfing_an_item },
    { "BR36818_open_area_vault_rejects_engulfing_monster",
      br36818_monster_setup, NULL, br36818_rejects_engulfing_a_monster },
    { "BR36818_open_area_vault_rejects_engulfing_another_vault",
      br36818_vault_setup, NULL, br36818_rejects_engulfing_another_vault },

    { NULL, NULL, NULL, NULL }
};
