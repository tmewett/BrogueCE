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
// built in, the shapes and placement of rooms other than chunky rooms,
// vault/lake shapes and placement, doorway and secret
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
    { "BR33256_pathfinding_avoids_damage_immune_stationary_monsters", NULL, NULL, damage_immune_stationary_monsters_are_pathfinding_obstacles },
    { "BR68830_dropped_item_lands_on_nearest_reachable_tile", br68830_drop_setup, br68830_drop_step, dropped_item_lands_on_nearest_reachable_tile },

    { "BR79876_gas_feature_adds_start_probability_to_volume", gas_volume_setup, gas_dissipates_step, stacked_gas_volume_outlasts_single },

    { "BR26376_vaults_purge_liquids_from_interiors", NULL, NULL, vaults_purge_liquids_from_their_interiors },

    { "BR26268_hallways_respect_length_bounds", NULL, NULL, hallways_respect_length_bounds },

    { "BR93781_bridges_meet_placement_conditions", NULL, NULL, bridges_meet_placement_conditions },

    { "BR44816_generated_levels_stay_connected",  NULL, NULL, br44816_levels_stay_connected },

    { "BR24979_lake_bodies_fit_the_generation_canvas", NULL, NULL, lake_bodies_fit_the_generation_canvas },

    { "BR88382_chunky_rooms_are_radius2_disc_chains", NULL, NULL, chunky_rooms_are_radius2_disc_chains },

    { NULL, NULL, NULL, NULL }
};
