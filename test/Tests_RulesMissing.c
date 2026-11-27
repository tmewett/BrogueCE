// Tests for rules listed in test/RulesMissingTests.txt.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"
#include <stdio.h>

// ============================================================
// BR27132 — Falling on the deepest level instead teleports you
// away with "A strange force seizes you as you fall."
// ============================================================

static pos br27132_start_pos;

static void br27132_setup(void) {
    clearArena();
    setDepth(dungeonDeepestLevel());
    setTerrainAroundLoc(player.loc, CHASM);
    setPlayerPos(player.loc);
    br27132_start_pos = getPlayerPos();
}

static boolean br27132_step(void) {
    if (messageWasShown("A strange force seizes you as you fall.")) {
        return false;
    }
    if (turnNumber() < 10) {
        restOneTurn();
        return true;
    }
    return false;
}

static boolean br27132_verify(void) {
    TEST_ASSERT(messageWasShown("A strange force seizes you as you fall."),
                "falling on the deepest level should show the strange force message");
    TEST_ASSERT(!playerIsAt(br27132_start_pos),
                "falling on the deepest level should teleport the player");
    return !testFailed;
}

// ============================================================
// BR26601 — An unidentified ring is named after a gemstone,
// and no two kinds of ring share the same gemstone in a given
// game.
// ============================================================

static const char *gemstone_names[] = {
    "diamond", "opal", "garnet", "ruby", "amethyst", "topaz",
    "onyx", "tourmaline", "sapphire", "obsidian", "malachite",
    "aquamarine", "emerald", "jade", "alexandrite", "agate",
    "bloodstone", "jasper"
};

#define GEMSTONE_COUNT 18

static item *br26601_rings[8];

static void br26601_setup(void) {
    clearArena();
    for (int i = 0; i < 8; i++) {
        br26601_rings[i] = givePlayerUnidentifiedItem(RING, i, 0);
    }
}

static boolean br26601_verify(void) {
    boolean seen[GEMSTONE_COUNT] = {false};

    for (int i = 0; i < 8; i++) {
        TEST_ASSERT(br26601_rings[i] != NULL, "setup should create a ring");
        if (br26601_rings[i] == NULL) {
            continue;
        }

        const char *text = itemExamineText(br26601_rings[i]);
        TEST_ASSERT(strstr(text, "RING") != NULL || strstr(text, "ring") != NULL,
                    "unidentified ring should be named as a ring");

        int found = -1;
        for (int j = 0; j < GEMSTONE_COUNT; j++) {
            if (strstr(text, gemstone_names[j]) != NULL) {
                if (found != -1) {
                    testFail("unidentified ring should contain exactly one gemstone name");
                    return false;
                }
                found = j;
            }
        }

        TEST_ASSERT(found != -1, "unidentified ring should contain a gemstone name");
        if (found == -1) {
            continue;
        }

        TEST_ASSERT(!seen[found], "no two ring kinds should share the same gemstone");
        seen[found] = true;
    }

    return !testFailed;
}



// ============================================================
// BR27784 — when a rebuilt vault ends up with a room sealed off
// from the rest of the level, the game tunnels that orphaned
// room back along the shortest orthogonal route, carving every
// rock tile on the way into floor so the room is reachable
// again.
//
// The fixture lays a block of solid vault interior with one
// already-open floor tile in a corner and the sealed room's door
// in the opposite corner, then runs the engine's reconnection
// pass.  The route it carves must be exactly the Manhattan
// distance between the two corners: a longer or diagonal route
// would leave a different number of floor tiles, and a pass that
// carved nothing would leave the door sealed in rock.
// ============================================================

static const int br27784_dx[4] = {0, 0, -1, 1};
static const int br27784_dy[4] = {-1, 1, 0, 0};

// How many floor tiles of the fixture's interior can be walked to from the
// open corner, moving orthogonally.
static int br27784_reachableFloors(pos open, pos corner) {
    boolean seen[DCOLS][DROWS];
    pos queue[DCOLS * DROWS];
    int head = 0, tail = 0, count = 0;

    memset(seen, 0, sizeof(seen));
    seen[open.x][open.y] = true;
    queue[tail++] = open;
    while (head < tail) {
        pos here = queue[head++];
        count++;
        for (int d = 0; d < 4; d++) {
            pos next = posOffset(here, br27784_dx[d], br27784_dy[d]);
            if (next.x < open.x || next.x > corner.x || next.y < open.y || next.y > corner.y
                || seen[next.x][next.y] || terrainAt(next) != FLOOR) {
                continue;
            }
            seen[next.x][next.y] = true;
            queue[tail++] = next;
        }
    }
    return count;
}

static boolean br27784_tunnel_is_the_shortest_route(int width, int height) {
    pos open = posAt(0, 0);
    pos orphan = posAt(0, 0);

    // Solid granite around the fixture, so the only way from the open floor to
    // the sealed room is the tunnel the pass carves.
    for (int x = 26; x <= 40; x++) {
        for (int y = 6; y <= 20; y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }

    TEST_ASSERT(buildOrphanedVaultInterior(width, height, &open, &orphan),
                "the vault-interior fixture should build");

    // Only the carved tunnel is floor; everything else in the block is rock.
    int floors = 0;
    for (int x = open.x; x <= orphan.x; x++) {
        for (int y = open.y; y <= orphan.y; y++) {
            if (terrainAt(posAt(x, y)) == FLOOR) {
                floors++;
            }
        }
    }

    TEST_ASSERT(terrainAt(orphan) == FLOOR,
                "the sealed room's door should be carved open");
    TEST_ASSERT_EQ(floors, width + height - 1,
                   "the tunnel should be the shortest orthogonal route between the corners");
    TEST_ASSERT_EQ(br27784_reachableFloors(open, orphan), floors,
                   "the tunnel should join the sealed room to the open floor");
    return !testFailed;
}

static boolean br27784_orphaned_room_is_reconnected(void) {
    br27784_tunnel_is_the_shortest_route(6, 4);
    br27784_tunnel_is_the_shortest_route(3, 5);
    return !testFailed;
}

// ============================================================
// BR34296 — every monster you can currently see is marked as
// having just been visible, the mark is dropped once you cannot
// see it any more, and that mark is what makes the monster-
// spotting alert fire when a monster first comes into view
// rather than on every turn it stays in sight.
//
// Cautious mode is the setting under which the game announces
// each monster it spots with "you see a goblin".  A paralyzed
// goblin stands in view for several turns: it must be announced
// on the turn it first comes into view, must not be announced
// again while it stays there, and must be announced again after
// it leaves sight and returns.
// ============================================================

static creature *br34296_goblin;
static pos br34296_inView;
static pos br34296_outOfView;

static void br34296_setup(void) {
    clearMonsters();
    clearFloorItems();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(20, FLOOR);
    br34296_inView = posOffset(getPlayerPos(), 2, 0);
    br34296_outOfView = posOffset(getPlayerPos(), 25, 0);

    br34296_goblin = placeHostileMonster(MK_GOBLIN, br34296_inView);
    TEST_ASSERT(br34296_goblin != NULL, "setup: the goblin should be placed");
    if (br34296_goblin != NULL) {
        // Paralyzed so it holds still and cannot interrupt the rests.
        applyStatusToMonster(br34296_goblin, STATUS_PARALYZED, 3000);
    }
    TEST_ASSERT(cellIsVisible(br34296_inView),
                "precondition: the goblin's square is in view");
}

static boolean br34296_alert_fires_once_per_arrival(void) {
    // The goblin first comes into view.
    clearMessages();
    enableCautiousMode();
    restOneTurn();
    TEST_ASSERT(messageWasShown("you see a goblin"),
                "a goblin coming into view should be announced");

    // It stays in view for two more turns.
    clearMessages();
    enableCautiousMode();
    restOneTurn();
    enableCautiousMode();
    restOneTurn();
    TEST_ASSERT(!messageWasShown("you see a goblin"),
                "a goblin already in view should not be announced again");

    // It leaves view, which must drop the mark.
    setMonsterPos(br34296_goblin, br34296_outOfView);
    clearMessages();
    restOneTurn();
    TEST_ASSERT(!cellIsVisible(br34296_outOfView),
                "precondition: the goblin should now be out of view");
    TEST_ASSERT(!messageWasShown("you see a goblin"),
                "a goblin leaving view should not be announced");

    // It comes back into view, so the mark must be gone.
    setMonsterPos(br34296_goblin, br34296_inView);
    clearMessages();
    enableCautiousMode();
    restOneTurn();
    TEST_ASSERT(messageWasShown("you see a goblin"),
                "a goblin returning to view should be announced again");
    return !testFailed;
}

// ============================================================
// BR19585 — an identified staff of blinking is limited to a
// fixed maximum blink distance.
//
// At enchantment two that distance is two plus twice two, six
// spaces: the aim line stops at the sixth space even when the
// cursor is twelve spaces away, and confirming that aim blinks
// you exactly those six spaces instead of to the aimed square.
// ============================================================

static const int br19585_aimDistance = 12;
static const int br19585_blinkDistance = 6;
static pos br19585_start;

static void br19585_setup(void) {
    clearMonsters();
    clearFloorItems();
    removePackItems(STAFF, STAFF_BLINKING);
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(br19585_aimDistance + 2, FLOOR);
    for (int i = 1; i <= br19585_aimDistance; i++) {
        revealCell(posOffset(getPlayerPos(), i, 0));
    }

    item *staff = givePlayerItem(STAFF, STAFF_BLINKING, 2);
    TEST_ASSERT(staff != NULL, "setup: the staff of blinking should be created");
    if (staff != NULL) {
        TEST_ASSERT(itemIsIdentified(staff),
                    "precondition: the staff of blinking should be identified");
        TEST_ASSERT(itemCharges(staff) > 0, "precondition: the staff should have a charge");
    }
    br19585_start = getPlayerPos();
}

static void br19585_aimTwelveSpacesEast(void) {
    for (int i = 0; i < br19585_aimDistance; i++) {
        queueKeystroke(RIGHT_KEY, false, false);
    }
}

static boolean br19585_locked_to_a_fixed_distance(void) {
    // Aim twelve spaces away without confirming: the line stops at six.
    br19585_aimTwelveSpacesEast();
    applyPackItem(STAFF, STAFF_BLINKING);
    rememberScreenColors();
    runQueuedInputThroughMainLoop();
    TEST_ASSERT(screenCellColorsChanged(posOffset(br19585_start, br19585_blinkDistance, 0)),
                "the aim line should reach the staff's maximum blink distance");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(br19585_start, br19585_blinkDistance + 1, 0)),
                "the aim line should stop at the staff's fixed maximum blink distance");

    // Confirm the same aim: the blink carries only those six spaces.
    br19585_aimTwelveSpacesEast();
    queueKeystroke(RETURN_KEY, false, false);
    applyPackItem(STAFF, STAFF_BLINKING);
    TEST_ASSERT_POS_EQ(getPlayerPos(), posOffset(br19585_start, br19585_blinkDistance, 0),
                       "confirming an aim beyond the limit should blink only the fixed maximum");
    return !testFailed;
}

// ============================================================
// BR13458 — typing a turn beyond the recording's total length
// jumps only as far as the recording's final turn, so the
// destination never exceeds the number of turns available.
//
// The fixture arms playback with eight recorded turns and declares
// the recording to end three turns ahead, then types 999 into the
// "Go to turn number:" prompt.  Eight step keystrokes are left
// queued: a capped seek consumes only the three turns it needs,
// while an uncapped seek would replay all eight and land five turns
// past the recording's end, so the turn count discriminates the two.
// ============================================================

#define BR13458_RECORDED_TURNS 8
#define BR13458_TURNS_TO_END 3

static int br13458_startTurn;
static int br13458_finalTurn;

static void br13458_setup(void) {
    clearArena();
    // The replayed steps must not land on a staircase.
    placeDownStairs(posAt(1, 1));
    placeUpStairs(posAt(levelWidth() - 2, levelHeight() - 2));
    startPlaybackWithRecordedChecks(BR13458_RECORDED_TURNS);
    br13458_startTurn = playerTurnCount();
    br13458_finalTurn = br13458_startTurn + BR13458_TURNS_TO_END;
    setRecordingFinalTurn(br13458_finalTurn);
}

static boolean br13458_step(void) {
    // A digit opens the "Go to turn number:" prompt; the other digits and
    // Return answer it.  999 lies far beyond the recording's end.
    queueKeystroke('9', false, false);
    queueKeystroke('9', false, false);
    queueKeystroke('9', false, false);
    queueKeystroke(RETURN_KEY, false, false);
    // The same alternating steps startPlaybackWithRecordedChecks() recorded,
    // so the seek can replay them; more than the capped distance is queued.
    for (int i = 0; i < BR13458_RECORDED_TURNS; i++) {
        queueKeystroke((i % 2) ? DOWNLEFT_KEY : UPRIGHT_KEY, false, false);
    }
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean br13458_stops_at_the_final_turn(void) {
    TEST_ASSERT(br13458_finalTurn > br13458_startTurn,
                "control: the recording should have turns left to replay");
    TEST_ASSERT_EQ(playerTurnCount(), br13458_finalTurn,
                   "typing a turn past the recording's end should stop at its final turn");
    return !testFailed;
}

// ============================================================
// BR20866 — every square on the dungeon grid has eight
// neighbours, and creatures can move, strike, and be caught by
// area effects in any of those directions.
//
// The eight directions are walked one step at a time on open
// floor, so each move must land exactly on the neighbouring
// square: a game limited to the four straight directions would
// leave the player in place on the diagonal moves.  A goblin is
// then placed in each neighbouring square in turn and given its
// turn; you stand web-stuck, which makes every blow land, and
// each goblin must wound you from its own direction.
// ============================================================

static const enum directions br20866_directions[8] = {
    UP, DOWN, LEFT, RIGHT, UPLEFT, DOWNLEFT, UPRIGHT, DOWNRIGHT
};
static const int br20866_dx[8] = { 0, 0, -1, 1, -1, -1, 1, 1 };
static const int br20866_dy[8] = { -1, 1, 0, 0, -1, 1, -1, 1 };

static void br20866_open_arena_setup(void) {
    clearMonsters();
    clearFloorItems();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(10, FLOOR);
}

static boolean br20866_moves_in_all_eight_directions(void) {
    for (int i = 0; i < 8; i++) {
        pos before = getPlayerPos();
        moveDirection(br20866_directions[i]);
        TEST_ASSERT_POS_EQ(getPlayerPos(), posOffset(before, br20866_dx[i], br20866_dy[i]),
                           "a creature should be able to step to each of a square's eight neighbours");
    }
    return !testFailed;
}

static void br20866_strike_setup(void) {
    clearMonsters();
    clearFloorItems();
    setArenaRadius(10, FLOOR);
    // Refresh the scent trail and field of view so the goblins hunt rather
    // than wander.
    applyStatusToPlayer(STATUS_AGGRAVATING, 10000);
    restOneTurn();
    // Stand in a spiderweb and web yourself in place so every blow lands on a
    // stuck defender, without the triple damage a paralyzed one would take.
    pos here = getPlayerPos();
    setTerrain(here, SPIDERWEB);
    setPlayerPos(here);
    applyStatusToPlayer(STATUS_STUCK, 1000);
}

static boolean br20866_strikes_in_all_eight_directions(void) {
    pos center = getPlayerPos();
    for (int i = 0; i < 8; i++) {
        clearMonsters();
        setPlayerHealth(playerMaxHealth());
        int healthBefore = playerHealth();
        creature *goblin = placeHostileMonster(MK_GOBLIN,
                                               posOffset(center, br20866_dx[i], br20866_dy[i]));
        TEST_ASSERT(goblin != NULL, "setup: the goblin should be placed in a neighbouring square");
        if (goblin == NULL) {
            continue;
        }
        TEST_ASSERT(monsterIsAdjacentToPlayer(goblin),
                    "every one of the eight neighbouring squares should count as adjacent");
        monsterTakesTurn(goblin);
        TEST_ASSERT(playerHealth() < healthBefore,
                    "a goblin in any neighbouring square should be able to strike you");
    }
    return !testFailed;
}

// ============================================================
// BR58109 — Ogre.  Health 55, defense 60, accuracy 125,
// damage 9–13; regenerates every 20 turns; normal movement but
// attacks at half rate (attack speed 200).  A large creature
// that pushes its victim backward on a hit and avoids corridors
// while hunting.
//
// The stat block, size, and the two ability flags are read from
// the spawned ogre.  The 20-turn regeneration period is watched
// turn by turn: a wounded ally with no enemies about can only
// regain health through regeneration, so it must sit at 1 HP for
// 19 turns and regain exactly one point on the 20th.
// ============================================================

static creature *br58109_ogre;
static int br58109_regenTurns;

static void br58109_setup(void) {
    clearMonsters();
    clearFloorItems();
    setArenaRadius(6, FLOOR);
    br58109_ogre = spawnMonster(MK_OGRE, posOffset(getPlayerPos(), 2, 0));
    makeMonsterAlly(br58109_ogre);
    setMonsterHealth(br58109_ogre, 1);
    br58109_regenTurns = 0;
}

static boolean br58109_step(void) {
    restOneTurn();
    br58109_regenTurns++;
    if (monsterHealth(br58109_ogre) > 1) {
        return false;
    }
    return br58109_regenTurns < 60;
}

static boolean br58109_stat_block_and_regeneration(void) {
    TEST_ASSERT(br58109_ogre != NULL, "ogre spawned");
    if (br58109_ogre == NULL) {
        return false;
    }
    TEST_ASSERT_EQ(monsterMaxHealth(br58109_ogre), 55, "ogre health");
    TEST_ASSERT_EQ(monsterBaseDefense(br58109_ogre), 60, "ogre defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(br58109_ogre), 125, "ogre accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(br58109_ogre), 9, "ogre damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(br58109_ogre), 13, "ogre damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(br58109_ogre), 100,
                   "the ogre moves at normal speed");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(br58109_ogre), 200,
                   "the ogre attacks at half rate");
    TEST_ASSERT(monsterRegenerates(br58109_ogre), "the ogre regenerates naturally");
    TEST_ASSERT(monsterIsLarge(br58109_ogre), "the ogre is large");
    TEST_ASSERT(monsterHasAbility(br58109_ogre, MA_ATTACKS_STAGGER),
                "the ogre pushes its victim backward");
    TEST_ASSERT(monsterHasAbility(br58109_ogre, MA_AVOID_CORRIDORS),
                "the ogre avoids corridors while hunting");
    TEST_ASSERT_EQ(br58109_regenTurns, 20,
                   "the ogre should wait 20 turns before regenerating");
    TEST_ASSERT_EQ(monsterHealth(br58109_ogre), 2,
                   "the ogre should regain exactly one point of health per period");
    return !testFailed;
}

// ============================================================
// BR13225 — An item's detail box offers an Equip or Remove
// button for armor, weapons, and rings.
// ============================================================

static void br13225_weapon_setup(void) {
    clearPack();
    givePlayerItem(WEAPON, DAGGER, 0);
}

static boolean br13225_weapon_equip_remove(void) {
    inventoryDetailHeldAfterKeys(DOWN_ARROW, DOWN_ARROW, 0);
    TEST_ASSERT(screenTextContains("equip"), "unequipped weapon should offer Equip");
    TEST_ASSERT(!screenTextContains("remove"), "unequipped weapon should not offer Remove");

    equipItemFromPack(WEAPON, DAGGER);
    inventoryDetailHeldAfterKeys(DOWN_ARROW, DOWN_ARROW, 0);
    TEST_ASSERT(screenTextContains("remove"), "equipped weapon should offer Remove");
    TEST_ASSERT(!screenTextContains("equip"), "equipped weapon should not offer Equip");
    return !testFailed;
}

static void br13225_armor_setup(void) {
    clearPack();
    givePlayerItem(ARMOR, LEATHER_ARMOR, 0);
}

static boolean br13225_armor_equip_remove(void) {
    inventoryDetailHeldAfterKeys(DOWN_ARROW, DOWN_ARROW, 0);
    TEST_ASSERT(screenTextContains("equip"), "unequipped armor should offer Equip");
    TEST_ASSERT(!screenTextContains("remove"), "unequipped armor should not offer Remove");

    equipItemFromPack(ARMOR, LEATHER_ARMOR);
    inventoryDetailHeldAfterKeys(DOWN_ARROW, DOWN_ARROW, 0);
    TEST_ASSERT(screenTextContains("remove"), "equipped armor should offer Remove");
    TEST_ASSERT(!screenTextContains("equip"), "equipped armor should not offer Equip");
    return !testFailed;
}

static void br13225_ring_setup(void) {
    clearPack();
    givePlayerItem(RING, RING_STEALTH, 0);
}

static boolean br13225_ring_equip_remove(void) {
    inventoryDetailHeldAfterKeys(DOWN_ARROW, DOWN_ARROW, 0);
    TEST_ASSERT(screenTextContains("equip"), "unequipped ring should offer Equip");
    TEST_ASSERT(!screenTextContains("remove"), "unequipped ring should not offer Remove");

    equipItemFromPack(RING, RING_STEALTH);
    inventoryDetailHeldAfterKeys(DOWN_ARROW, DOWN_ARROW, 0);
    TEST_ASSERT(screenTextContains("remove"), "equipped ring should offer Remove");
    TEST_ASSERT(!screenTextContains("equip"), "equipped ring should not offer Equip");
    return !testFailed;
}

static void br13225_potion_setup(void) {
    clearPack();
    givePlayerItem(POTION, POTION_LIFE, 0);
}

static boolean br13225_potion_no_equip_remove(void) {
    inventoryDetailHeldAfterKeys(DOWN_ARROW, DOWN_ARROW, 0);
    TEST_ASSERT(!screenTextContains("equip"), "potion should not offer Equip");
    TEST_ASSERT(!screenTextContains("remove"), "potion should not offer Remove");
    TEST_ASSERT(screenTextContains("apply"), "potion should offer Apply");
    return !testFailed;
}

// ============================================================
// BR37960 — A darkened crystal globe keeps its mechanism from
// activating until an electric shock lights it, and the
// shining globe no longer breaks the circuit.
// ============================================================

static pos br37960_globeLoc;
static pos br37960_pipeLoc;

static void br37960_setup(void) {
    clearMonsters();
    clearFloorItems();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(10, FLOOR);
    br37960_globeLoc = posOffset(getPlayerPos(), 2, 0);
    br37960_pipeLoc = posOffset(getPlayerPos(), 3, 0);
}

static boolean br37960_darkened_globe_blocks_mechanism(void) {
    placeGlobeAndPipeWiredToMechanism(br37960_globeLoc, br37960_pipeLoc, false);
    powerMechanismIncluding(br37960_globeLoc);
    TEST_ASSERT(cellIsGlowingGlassPipe(br37960_pipeLoc),
                "darkened globe should block mechanism activation");
    return !testFailed;
}

static boolean br37960_shining_globe_does_not_block(void) {
    placeGlobeAndPipeWiredToMechanism(br37960_globeLoc, br37960_pipeLoc, true);
    powerMechanismIncluding(br37960_globeLoc);
    TEST_ASSERT(cellIsCharredGlassPipe(br37960_pipeLoc),
                "shining globe should allow mechanism activation");
    return !testFailed;
}

static boolean br37960_shock_promotes_globe(void) {
    placeGlobeAndPipeWiredToMechanism(br37960_globeLoc, br37960_pipeLoc, false);
    TEST_ASSERT(cellIsTerrain(br37960_globeLoc, ELECTRIC_CRYSTAL_OFF),
                "globe should start darkened");
    TEST_ASSERT(exposeGlobeToElectricShock(br37960_globeLoc),
                "electric shock should promote the globe");
    TEST_ASSERT(cellIsTerrain(br37960_globeLoc, ELECTRIC_CRYSTAL_ON),
                "globe should be shining after the shock");
    powerMechanismIncluding(br37960_globeLoc);
    TEST_ASSERT(cellIsCharredGlassPipe(br37960_pipeLoc),
                "shock should let the mechanism activate");
    return !testFailed;
}

// ============================================================
// BR38379 — Activating a portal floods the level with light
// out to the full width of the level, with the light fading
// with distance from the portal.
// ============================================================

static void br38379_setup(void) {
    clearMonsters();
    clearFloorItems();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(levelWidth(), FLOOR);
}

static boolean br38379_portal_light_floods_level(void) {
    pos portalLoc = posAt(levelWidth() / 2, levelHeight() / 2);
    placePortalWiredToMechanism(portalLoc);
    powerMechanismIncluding(portalLoc);
    recomputeLighting();

    int lightAtPortal = lightAt(portalLoc);
    pos farEast = posAt(levelWidth() - 1, levelHeight() / 2);
    int lightAtFarEast = lightAt(farEast);

    TEST_ASSERT(lightAtPortal > lightAtFarEast,
                "light should fade with distance from the portal");
    TEST_ASSERT(lightAtFarEast > 0,
                "portal activation should flood the level with light");
    return !testFailed;
}

// ============================================================
// BR31620 — A sleeping monster wakes as soon as it takes
// damage from any source.
// ============================================================

static creature *br31620_sleeper;
static int br31620_healthBefore;

static void br31620_setup(void) {
    clearArena();
    // Damage is applied without an attacker, so this covers a source other
    // than a melee blow.
    br31620_sleeper = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    setMonsterHealth(br31620_sleeper, 50);
    putMonsterToSleep(br31620_sleeper);
    br31620_healthBefore = monsterHealth(br31620_sleeper);
}

static boolean br31620_wakes_when_damaged(void) {
    TEST_ASSERT(monsterIsAsleep(br31620_sleeper),
                "the monster should start out asleep");

    damageMonster(br31620_sleeper, 1);

    TEST_ASSERT(monsterHealth(br31620_sleeper) < br31620_healthBefore,
                "the damage should actually reduce the monster's health");
    TEST_ASSERT(!monsterIsAsleep(br31620_sleeper),
                "a sleeping monster should wake as soon as it takes damage");
    return !testFailed;
}

// ============================================================
// BR30195 — A scroll of remove curse removes the curse from
// every cursed item you carry, and announces whether anything
// changed.
// ============================================================

static item *br30195_weapon;
static item *br30195_armor;
static item *br30195_ring;
static item *br30195_potion;

static void br30195_cursed_pack_setup(void) {
    clearPack();
    br30195_weapon = givePlayerItem(WEAPON, DAGGER, 0);
    br30195_armor = givePlayerItem(ARMOR, LEATHER_ARMOR, 0);
    br30195_ring = givePlayerItem(RING, RING_STEALTH, 0);
    br30195_potion = givePlayerItem(POTION, POTION_STRENGTH, 0);

    equipItemFromPack(WEAPON, DAGGER);
    curseItem(br30195_weapon);
    curseItem(br30195_armor);
    curseItem(br30195_ring);
}

static boolean br30195_cleanses_every_cursed_item(void) {
    TEST_ASSERT(itemIsCursed(br30195_weapon)
                && itemIsCursed(br30195_armor)
                && itemIsCursed(br30195_ring),
                "the weapon, armor, and ring should start cursed");
    TEST_ASSERT(!itemIsCursed(br30195_potion),
                "the potion should start uncursed");

    readScrollOfKind(SCROLL_REMOVE_CURSE);

    TEST_ASSERT(!itemIsCursed(br30195_weapon),
                "remove curse should uncurse the weapon, including an equipped one");
    TEST_ASSERT(!itemIsCursed(br30195_armor),
                "remove curse should uncurse the armor");
    TEST_ASSERT(!itemIsCursed(br30195_ring),
                "remove curse should uncurse the ring");
    TEST_ASSERT(!itemIsCursed(br30195_potion),
                "remove curse should leave an already-uncursed item alone");
    TEST_ASSERT(messageWasShown("malevolent energy disperses"),
                "remove curse should announce that it lifted a curse");
    return !testFailed;
}

static void br30195_clean_pack_setup(void) {
    clearPack();
    br30195_potion = givePlayerItem(POTION, POTION_STRENGTH, 0);
}

static boolean br30195_announces_nothing_changed(void) {
    TEST_ASSERT(!itemIsCursed(br30195_potion),
                "the only carried item should start uncursed");

    readScrollOfKind(SCROLL_REMOVE_CURSE);

    TEST_ASSERT(!messageWasShown("malevolent energy disperses"),
                "remove curse should not announce a curse it did not lift");
    TEST_ASSERT(messageWasShown("nothing happens"),
                "remove curse should announce that nothing changed");
    return !testFailed;
}

// ============================================================
// BR72559 — A thrown benign potion splashes harmlessly at the
// impact point.
// ============================================================

static item *br72559_potion;
static int br72559_strengthBefore;
static int br72559_potionsBefore;

static void br72559_setup(void) {
    clearArena();
    clearFloorItems();
    clearPack();
    br72559_potion = givePlayerItem(POTION, POTION_STRENGTH, 0);
    br72559_strengthBefore = playerStrength();
    br72559_potionsBefore = packUnitCount(POTION, POTION_STRENGTH);
    queueKeystroke(itemInventoryLetter(br72559_potion), false, false);
    queueKeystroke(RIGHT_KEY, false, false);
    queueKeystroke(RIGHT_KEY, false, false);
    queueKeystroke(RETURN_KEY, false, false); // confirm empty floor as the target
}

static boolean br72559_step(void) {
    injectKeystroke(THROW_KEY, false, false);
    return false;
}

static boolean br72559_splashes_harmlessly(void) {
    TEST_ASSERT(messageWasShown("splashes harmlessly"),
                "a thrown benign potion should splash harmlessly at the impact point");
    TEST_ASSERT_EQ(packUnitCount(POTION, POTION_STRENGTH), br72559_potionsBefore - 1,
                   "the thrown potion should leave the pack");
    TEST_ASSERT(findFloorItem(POTION, POTION_STRENGTH) == NULL,
                "a shattered potion should not land on the floor");
    TEST_ASSERT_EQ(playerStrength(), br72559_strengthBefore,
                   "a thrown benign potion should have no effect on the player");
    return !testFailed;
}

// ============================================================
// BR16761 — Before a route is calculated every square's stored
// distance is wiped back to a fixed maximum of 30000, so no
// distance from an earlier calculation carries over and a
// square that cannot reach the destination has no route.
// ============================================================

#define BR16761_NO_ROUTE 30000

static boolean br16761_unreachable_square_keeps_maximum(void) {
    clearMonsters();
    pos base = getPlayerPos();
    pos outside = posOffset(base, -2, 0);
    pos inside = posOffset(base, -4, 0);

    // A short open corridor through which the inner tile can be reached.
    for (int x = -5; x <= -2; x++) {
        for (int dy = -1; dy <= 1; dy++) {
            setTerrain(posOffset(base, x, dy), FLOOR);
        }
    }

    TEST_ASSERT_EQ(pathingDistanceBetween(inside, outside, 0), 2,
                   "control: the two tiles are two steps apart in the open corridor");
    TEST_ASSERT_EQ(pathingDistanceBetween(inside, base, 0), 4,
                   "control: the inner tile reaches the player's square in four steps");

    // Seal the inner tile behind a ring of granite.
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx != 0 || dy != 0) {
                setTerrain(posOffset(inside, dx, dy), GRANITE);
            }
        }
    }

    TEST_ASSERT_EQ(pathingDistanceBetween(inside, outside, 0), BR16761_NO_ROUTE,
                   "a square that cannot reach the destination keeps the maximum distance of 30000");
    TEST_ASSERT_EQ(pathingDistanceBetween(inside, base, 0), BR16761_NO_ROUTE,
                   "no distance from the earlier calculation may survive the sealed-off route");
    TEST_ASSERT_EQ(pathingDistanceBetween(outside, base, 0), 2,
                   "reachable squares keep their real route length");

    // Opening the east side of the seal restores a two-step route.
    setTerrain(posOffset(inside, 1, 0), FLOOR);
    TEST_ASSERT_EQ(pathingDistanceBetween(inside, outside, 0), 2,
                   "rolling back the seal restores the real route length");
    return !testFailed;
}

// ============================================================
// BR31244 — Every 100 ticks every monster feels instantaneous
// tile effects, whether or not its own turn has come.
//
// Two paralyzed monsters cannot take a turn, so the only way
// they can be confused is the 100-tick environment update.
// ============================================================

static creature *br31244_monsters[2];
static pos br31244_locs[2];

static void br31244_setup(void) {
    clearArena();
    clearMonsters();
    pos base = getPlayerPos();

    for (int i = 0; i < 2; i++) {
        br31244_locs[i] = i == 0 ? posOffset(base, 2, 0) : posOffset(base, 0, 2);
        setTerrain(br31244_locs[i], FLOOR);
        br31244_monsters[i] = spawnMonster(MK_RAT, br31244_locs[i]);
        applyStatusToMonster(br31244_monsters[i], STATUS_PARALYZED, 100);
        setGasAt(br31244_locs[i], quickDissipatingGasType(), 100);
        TEST_ASSERT(!monsterHasStatus(br31244_monsters[i], STATUS_CONFUSED),
                    "control: a monster is not confused before any 100-tick update");
    }
}

static boolean br31244_rest_one_turn_step(void) {
    restOneTurn();
    return false;
}

static boolean br31244_idle_monsters_feel_the_gas(void) {
    for (int i = 0; i < 2; i++) {
        TEST_ASSERT(monsterIsAlive(br31244_monsters[i]),
                    "stand-in monster should survive the update");
        TEST_ASSERT_POS_EQ(monsterPosition(br31244_monsters[i]), br31244_locs[i],
                           "the paralyzed monster should not take a turn of its own");
        TEST_ASSERT(monsterHasStatus(br31244_monsters[i], STATUS_CONFUSED),
                    "every monster, even one that took no turn, should feel the confusion gas at the 100-tick update");
    }
    return !testFailed;
}

// ============================================================
// BR39133 — In the CSV seed catalog each item is split into
// separate category, kind, rune and enchantment fields; an
// enchantable item's enchantment field is its enchantment level,
// except a wand's, which is its remaining charge count.
// ============================================================

static item *br39133_sword;
static item *br39133_armor;
static item *br39133_ring;
static item *br39133_staff;
static item *br39133_wand;
static item *br39133_charm;
static item *br39133_potion;
static item *br39133_runicSword;

static void br39133_setup(void) {
    clearArena();
    clearPack();
    br39133_sword = givePlayerPlainWeapon(SWORD, 2);
    br39133_armor = givePlayerPlainArmor(LEATHER_ARMOR, 3);
    br39133_ring = givePlayerItem(RING, RING_CLAIRVOYANCE, 2);
    br39133_staff = givePlayerItem(STAFF, STAFF_BLINKING, 4);
    br39133_wand = givePlayerItem(WAND, WAND_TELEPORT, 2);
    setItemCharges(br39133_wand, 7);
    br39133_charm = givePlayerItem(CHARM, 0, 3);
    br39133_potion = givePlayerItem(POTION, POTION_LIFE, 0);
    br39133_runicSword = givePlayerRunicWeapon(SWORD, W_SPEED, 5, true);
}

static void br39133_expectFields(item *theItem, const char *what,
                                 const char *category, const char *kind,
                                 const char *enchantment, const char *runic) {
    seedCatalogItemFields fields;
    char message[240];

    TEST_ASSERT(theItem != NULL, "setup should have created the item");
    if (theItem == NULL || !seedCatalogCsvFieldsForItem(theItem, &fields)) {
        return;
    }

    snprintf(message, sizeof(message), "%s: category field (got \"%s\", want \"%s\")",
             what, fields.category, category);
    TEST_ASSERT(strcmp(fields.category, category) == 0, message);

    snprintf(message, sizeof(message), "%s: kind field (got \"%s\", want \"%s\")",
             what, fields.kind, kind);
    TEST_ASSERT(strcmp(fields.kind, kind) == 0, message);

    snprintf(message, sizeof(message), "%s: enchantment field (got \"%s\", want \"%s\")",
             what, fields.enchantment, enchantment);
    TEST_ASSERT(strcmp(fields.enchantment, enchantment) == 0, message);

    snprintf(message, sizeof(message), "%s: runic field (got \"%s\", want \"%s\")",
             what, fields.runic, runic);
    TEST_ASSERT(strcmp(fields.runic, runic) == 0, message);
}

// Category, kind and rune are separate fields: a plain weapon and a runic
// weapon of the same kind share a kind field while only the runic one names
// a rune.
static boolean br39133_category_kind_and_rune_are_separate(void) {
    br39133_expectFields(br39133_sword, "plain sword", "weapon", "sword", "2", "");
    br39133_expectFields(br39133_runicSword, "runic sword", "weapon", "sword", "5", "speed");
    return !testFailed;
}

// Enchantable items report their enchantment level; a wand reports its
// remaining charges instead; an item that cannot be enchanted gets no
// enchantment field at all.
static boolean br39133_enchantment_field(void) {
    br39133_expectFields(br39133_armor, "leather armor", "armor", "leather armor", "3", "");
    br39133_expectFields(br39133_ring, "ring of clairvoyance", "ring", "clairvoyance", "2", "");
    br39133_expectFields(br39133_charm, "charm of health", "charm", "health", "3", "");
    br39133_expectFields(br39133_staff, "staff of blinking", "staff", "blinking", "4", "");

    // A wand's enchantment field is its charges, not its enchantment level.
    TEST_ASSERT(br39133_wand != NULL, "setup should have created the wand");
    if (br39133_wand != NULL) {
        TEST_ASSERT_EQ(itemEnchant(br39133_wand), 2,
                       "control: the wand's enchantment level is 2");
        TEST_ASSERT_EQ(itemCharges(br39133_wand), 7,
                       "control: the wand has 7 charges left");
        br39133_expectFields(br39133_wand, "wand of teleportation", "wand", "teleportation", "7", "");
    }

    br39133_expectFields(br39133_potion, "potion of life", "potion", "life", "", "");
    return !testFailed;
}

// ============================================================
// BR12951 — a quotation mark immediately before a period or
// comma is swapped with it, so the punctuation appears inside
// the closing quote, in the American style.
//
// The messages run through the real archive and formatting pass.
// A formatter that left them alone would show the punctuation
// outside the closing quote, so each check separates the two.
// ============================================================

static boolean br12951_punctuation_moves_inside_closing_quote(void) {
    emitArchivedMessage("he said \"wait\".");
    const char *line = renderedMessageLine();
    TEST_ASSERT(strstr(line, "\"wait.\"") != NULL,
                "a period after a closing quote should be swapped inside the quote");
    TEST_ASSERT(strstr(line, "\"wait\".") == NULL,
                "the period should not remain outside the closing quote");

    emitArchivedMessage("he called \"hello\", then left");
    line = renderedMessageLine();
    TEST_ASSERT(strstr(line, "\"hello,\"") != NULL,
                "a comma after a closing quote should be swapped inside the quote");
    TEST_ASSERT(strstr(line, "\"hello\",") == NULL,
                "the comma should not remain outside the closing quote");
    return !testFailed;
}

// ============================================================
// BR30507 — a new seeded game prompts "Generate dungeon with
// seed number:" and accepts up to 20 digits.
//
// The prompt opens through the real main-menu path and its entry
// starts empty.  A twenty-digit seed must come through exactly;
// a twenty-five-digit entry must stop at the twentieth digit, so
// the same seed results.  A shorter or longer limit accepts a
// different number and fails the seed check.
// ============================================================

#define BR30507_TWENTY_DIGITS "12345678901234567890"
#define BR30507_TWENTY_FIVE_DIGITS "1234567890123456789012345"

static boolean br30507_seed_prompt_accepts_twenty_digits(void) {
    startSeededGameByTyping(BR30507_TWENTY_DIGITS);
    TEST_ASSERT(screenTextContains("Generate dungeon with seed number:"),
                "a new seeded game should prompt for a seed number");
    TEST_ASSERT(dungeonSeed() == 12345678901234567890ULL,
                "the seed prompt should accept a full twenty-digit seed");

    startSeededGameByTyping(BR30507_TWENTY_FIVE_DIGITS);
    TEST_ASSERT(dungeonSeed() == 12345678901234567890ULL,
                "the seed prompt should keep only the first twenty digits");
    return !testFailed;
}

// ============================================================
// BR59674 — a confused player who attacks an allied monster
// does not cause that ally to stop being allied.
//
// The ally is adjacent and has far more health than the blows
// can remove, so only the alliance can change.  Every landed
// blow is checked: the attack must connect, and the monster must
// still be an ally afterwards.  An implementation that drops the
// confusion exemption would end the alliance on the first hit.
// ============================================================

static creature *br59674_ally;

static void br59674_setup(void) {
    clearArena();
    br59674_ally = spawnMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    TEST_ASSERT(br59674_ally != NULL, "setup: the ally should spawn");
    if (br59674_ally != NULL) {
        makeMonsterAlly(br59674_ally);
        setMonsterHealth(br59674_ally, 1000);
    }
    applyStatusToPlayer(STATUS_CONFUSED, 100);
    TEST_ASSERT(br59674_ally != NULL && monsterIsAlly(br59674_ally),
                "precondition: the monster should be your ally");
    TEST_ASSERT(isPlayerConfused(), "precondition: you should be confused");
}

static boolean br59674_confused_attacks_keep_alliance(void) {
    if (br59674_ally == NULL) {
        return false;
    }
    TEST_ASSERT(monsterIsAlly(br59674_ally),
                "precondition: the ally should still be allied");
    TEST_ASSERT(isPlayerConfused(),
                "precondition: you should still be confused");

    for (int landed = 0; landed < 3; landed++) {
        boolean hit = false;
        for (int attempt = 0; attempt < 100 && !hit; attempt++) {
            hit = playerAttacksMonster(br59674_ally);
        }
        TEST_ASSERT(hit, "a melee attack against the adjacent ally should land");
        TEST_ASSERT(monsterIsAlive(br59674_ally),
                    "the ally should survive the blows under test");
        TEST_ASSERT(monsterIsAlly(br59674_ally),
                    "an ally you attack while confused should remain your ally");
        if (testFailed) {
            break;
        }
    }
    return !testFailed;
}

// ============================================================
// BR10073 — a wandering monster that notices you while you are
// in its field of view starts tracking you.
//
// The monster is caged behind eight closed portcullises: they
// block movement but not sight, so a wandering monster stays in
// the player's field of view and cannot reach or attack the
// player.  Each monster turn has a chance to notice the player;
// the first such turn must switch it from wandering to tracking.
// ============================================================

#define BR10073_MAX_TURNS 300

static creature *br10073_subject;
static pos br10073_monsterLoc;
static int br10073_turns;
static boolean br10073_startedWandering;
static boolean br10073_monsterInView;
static boolean br10073_wanderedInView;
static boolean br10073_trackedFromWanderInView;

static void br10073_setup(void) {
    clearArena();
    setPlayerHealth(100000);
    br10073_monsterLoc = posOffset(getPlayerPos(), 3, 0);
    setTerrain(br10073_monsterLoc, FLOOR);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx != 0 || dy != 0) {
                setTerrain(posOffset(br10073_monsterLoc, dx, dy), PORTCULLIS_CLOSED);
            }
        }
    }
    br10073_subject = spawnMonster(MK_RAT, br10073_monsterLoc);
    makeMonsterUnaware(br10073_subject);
    recomputeVision();

    br10073_startedWandering = monsterIsWandering(br10073_subject);
    br10073_monsterInView = cellIsInFieldOfView(monsterPosition(br10073_subject));
    br10073_turns = 0;
    br10073_wanderedInView = false;
    br10073_trackedFromWanderInView = false;
}

static boolean br10073_step(void) {
    if (br10073_turns >= BR10073_MAX_TURNS) {
        return false;
    }
    boolean wanderingBefore = monsterIsWandering(br10073_subject);
    boolean inViewBefore = cellIsInFieldOfView(monsterPosition(br10073_subject));
    boolean playerInView = cellIsInFieldOfView(getPlayerPos());

    monsterTakesTurn(br10073_subject);
    br10073_turns++;

    if (wanderingBefore && inViewBefore && playerInView) {
        br10073_wanderedInView = true;
    }
    if (monsterIsTrackingPlayer(br10073_subject)) {
        br10073_trackedFromWanderInView = wanderingBefore && inViewBefore && playerInView;
        return false;
    }
    return true;
}

static boolean br10073_wandering_monster_starts_tracking(void) {
    TEST_ASSERT(br10073_subject != NULL, "setup: the monster should spawn");
    TEST_ASSERT(br10073_startedWandering,
                "setup: the monster should start out wandering");
    TEST_ASSERT(br10073_monsterInView,
                "setup: the caged monster should be in your field of view");
    TEST_ASSERT(br10073_wanderedInView,
                "the wandering monster should spend turns in your field of view");
    TEST_ASSERT(monsterIsTrackingPlayer(br10073_subject),
                "a wandering monster that notices you in its field of view should start tracking you");
    TEST_ASSERT(br10073_trackedFromWanderInView,
                "tracking should begin on a turn when the monster was wandering and you were in its field of view");
    return !testFailed;
}

// ============================================================
// BR27525 — a distance map adds each square's entry cost, so a
// route prefers cheaper squares and detours around an expensive
// one.
//
// Two routes connect the ends: a direct two-step corridor and a
// four-step detour.  With equal costs the direct route wins by
// two; making the middle direct square cost ten makes the four-
// step detour cheaper; sealing the detour leaves the expensive
// direct route, whose distance is the sum of its entry costs.
// ============================================================

static pos br27525_a;
static pos br27525_mid;
static pos br27525_b;

static void br27525_setup(void) {
    br27525_a = posAt(20, 14);
    br27525_mid = posAt(21, 14);
    br27525_b = posAt(22, 14);

    // A walled chamber around the two routes.
    for (int x = 19; x <= 23; x++) {
        for (int y = 12; y <= 15; y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }
    // The direct route: A - mid - B.
    setTerrain(br27525_a, FLOOR);
    setTerrain(br27525_mid, FLOOR);
    setTerrain(br27525_b, FLOOR);
    // The longer route: A - (20,13) - (21,13) - (22,13) - B.
    setTerrain(posAt(20, 13), FLOOR);
    setTerrain(posAt(21, 13), FLOOR);
    setTerrain(posAt(22, 13), FLOOR);
}

static boolean br27525_entry_costs_add_to_route_distance(void) {
    TEST_ASSERT_EQ(pathingDistanceWithEntryCost(br27525_a, br27525_b, br27525_mid, 1), 2,
                   "control: with equal entry costs the two-step route is the shortest");

    TEST_ASSERT_EQ(pathingDistanceWithEntryCost(br27525_a, br27525_b, br27525_mid, 10), 4,
                   "a route should detour around an expensive square when the detour is cheaper");

    // Remove the detour, leaving only the expensive direct route.
    setTerrain(posAt(20, 13), GRANITE);
    TEST_ASSERT_EQ(pathingDistanceWithEntryCost(br27525_a, br27525_b, br27525_mid, 10), 11,
                   "the route distance should be the sum of its squares' entry costs");
    return !testFailed;
}

// ============================================================
// BR40479 — a monster that stops being invisible while it is in
// your view reappears on the map.
//
// The invisible, paralyzed monster stands beside the player: it
// cannot move, and its tile is drawn as bare floor while it is
// unseen.  When its one-turn invisibility expires, the same tile
// must be redrawn showing the monster.
// ============================================================

static creature *br40479_subject;
static boolean br40479_shownWhileInvisible;

static void br40479_setup(void) {
    clearArena();
    br40479_subject = spawnMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    setMonsterHealth(br40479_subject, 1000);
    applyStatusToMonster(br40479_subject, STATUS_PARALYZED, 1000);
    applyStatusToMonster(br40479_subject, STATUS_INVISIBLE, 1);
    // Draw the tile through the game's own appearance path while the
    // monster is invisible, as the map would already show it.
    revealCell(monsterPosition(br40479_subject));
    br40479_shownWhileInvisible = monsterIsShownOnMap(br40479_subject);
}

static boolean br40479_rest_one_turn_step(void) {
    restOneTurn();
    return false;
}

static boolean br40479_invisible_monster_reappears(void) {
    TEST_ASSERT(br40479_subject != NULL, "setup: the monster should spawn");
    TEST_ASSERT(!br40479_shownWhileInvisible,
                "an invisible monster should not be drawn on the map");
    TEST_ASSERT(!monsterHasStatus(br40479_subject, STATUS_INVISIBLE),
                "the monster's invisibility should have expired");
    TEST_ASSERT(cellIsVisible(monsterPosition(br40479_subject)),
                "the monster should still be in your view when its invisibility ends");
    TEST_ASSERT(monsterIsShownOnMap(br40479_subject),
                "a monster that stops being invisible while in your view should reappear on the map");
    return !testFailed;
}

const testCase tests_rulesMissing[] = {
    { "BR27132_fall_on_deepest_level", br27132_setup, br27132_step, br27132_verify },
    { "BR26601_ring_gemstones",        br26601_setup, NULL,          br26601_verify },
    { "BR27784_rebuilt_vault_tunnels_orphaned_room",   NULL, NULL, br27784_orphaned_room_is_reconnected },
    { "BR34296_spotting_alert_fires_once_per_arrival", br34296_setup, NULL, br34296_alert_fires_once_per_arrival },
    { "BR19585_identified_blink_staff_has_fixed_range", br19585_setup, NULL, br19585_locked_to_a_fixed_distance },
    { "BR13458_seek_stops_at_recording_end", br13458_setup, br13458_step, br13458_stops_at_the_final_turn },
    { "BR20866_moves_in_all_eight_directions", br20866_open_arena_setup, NULL, br20866_moves_in_all_eight_directions },
    { "BR20866_strikes_in_all_eight_directions", br20866_strike_setup, NULL, br20866_strikes_in_all_eight_directions },
    { "BR58109_ogre_stat_block_and_regeneration", br58109_setup, br58109_step, br58109_stat_block_and_regeneration },
    { "BR13225_weapon_equip_remove_buttons", br13225_weapon_setup, NULL, br13225_weapon_equip_remove },
    { "BR13225_armor_equip_remove_buttons", br13225_armor_setup, NULL, br13225_armor_equip_remove },
    { "BR13225_ring_equip_remove_buttons", br13225_ring_setup, NULL, br13225_ring_equip_remove },
    { "BR13225_potion_no_equip_remove_buttons", br13225_potion_setup, NULL, br13225_potion_no_equip_remove },
    { "BR37960_darkened_globe_blocks_mechanism", br37960_setup, NULL, br37960_darkened_globe_blocks_mechanism },
    { "BR37960_shining_globe_does_not_block", br37960_setup, NULL, br37960_shining_globe_does_not_block },
    { "BR37960_shock_promotes_globe", br37960_setup, NULL, br37960_shock_promotes_globe },
    { "BR38379_portal_light_floods_level", br38379_setup, NULL, br38379_portal_light_floods_level },
    { "BR31620_sleeping_monster_wakes_when_damaged", br31620_setup, NULL, br31620_wakes_when_damaged },
    { "BR30195_remove_curse_lifts_every_curse", br30195_cursed_pack_setup, NULL, br30195_cleanses_every_cursed_item },
    { "BR30195_remove_curse_announces_nothing_changed", br30195_clean_pack_setup, NULL, br30195_announces_nothing_changed },
    { "BR72559_thrown_benign_potion_splashes_harmlessly", br72559_setup, br72559_step, br72559_splashes_harmlessly },
    { "BR16761_unreachable_square_keeps_maximum_distance", NULL, NULL, br16761_unreachable_square_keeps_maximum },
    { "BR31244_idle_monsters_feel_instant_tile_effects", br31244_setup, br31244_rest_one_turn_step, br31244_idle_monsters_feel_the_gas },
    { "BR39133_csv_fields_split_category_kind_rune", br39133_setup, NULL, br39133_category_kind_and_rune_are_separate },
    { "BR39133_csv_enchantment_field", br39133_setup, NULL, br39133_enchantment_field },
    { "BR12951_quote_punctuation_moves_inside", NULL, NULL, br12951_punctuation_moves_inside_closing_quote },
    { "BR30507_seed_prompt_accepts_twenty_digits", NULL, NULL, br30507_seed_prompt_accepts_twenty_digits },
    { "BR59674_confused_attack_keeps_alliance", br59674_setup, NULL, br59674_confused_attacks_keep_alliance },
    { "BR10073_wandering_monster_starts_tracking", br10073_setup, br10073_step, br10073_wandering_monster_starts_tracking },
    { "BR27525_entry_costs_add_to_route_distance", br27525_setup, NULL, br27525_entry_costs_add_to_route_distance },
    { "BR40479_invisible_monster_reappears", br40479_setup, br40479_rest_one_turn_step, br40479_invisible_monster_reappears },
    { NULL, NULL, NULL, NULL }
};
