// Tests derived from rules/06-progression-score.md.
//
// This cover the deterministic, headless-observable progression rules: the
// state a new game starts in (depth, gold, turn counts, strength, starting
// equipment), the message log and archive being cleared and the screen
// blacked out before the welcome message, the reduced damage taken in easy
// mode, the income-tax announcement made when easy mode is enabled, the amulet
// fallback, the high-score gate on victory, the treasure
// tally a victory records, the high-score entry saved and displayed on death
// outside playback, and the text of that entry.
//
// Omitted are everything that depends on the run history, the achievement
// screens, stair travel between levels, and level restoration.  The wizard
// item and monster creation dialogs are driven headlessly from queued
// keystrokes and dungeon-tile clicks.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static int before;

// ============================================================
// Starting state
// ============================================================

static boolean new_game_begins_on_depth_1(void) {
    TEST_ASSERT_EQ(currentDepth(), 1,
                   "a new game should begin on depth 1");
    TEST_ASSERT_EQ(deepestDepth(), 1,
                   "a new game should record depth 1 as the deepest reached");
    return !testFailed;
}

static boolean new_game_starts_with_no_gold(void) {
    TEST_ASSERT_EQ(playerGold(), 0,
                   "a new game should start with no gold in hand");
    return !testFailed;
}

static boolean new_game_starts_at_turn_zero(void) {
    TEST_ASSERT_EQ(playerTurnCount(), 0,
                   "a new game should start at player turn 0");
    TEST_ASSERT_EQ((int)turnNumber(), 0,
                   "a new game should start at absolute turn 0");
    return !testFailed;
}

static boolean player_starts_with_12_strength(void) {
    TEST_ASSERT_EQ(playerStrength(), 12,
                   "the player should start with 12 strength");
    return !testFailed;
}

static boolean player_starts_with_food_ration(void) {
    TEST_ASSERT(playerHasItem(FOOD, RATION),
                "the player should start with a food ration");
    return !testFailed;
}

static boolean player_starts_with_dagger_equipped(void) {
    item *dagger = equippedWeapon();
    TEST_ASSERT(dagger != NULL, "the player should start with a weapon equipped");
    if (dagger == NULL) {
        return !testFailed;
    }
    TEST_ASSERT_EQ(itemKind(dagger), DAGGER,
                   "the starting weapon should be a dagger");
    TEST_ASSERT(!itemIsCursed(dagger),
                "the starting dagger should not be cursed");
    TEST_ASSERT(!itemIsRunic(dagger),
                "the starting dagger should not be runic");
    return !testFailed;
}

static boolean player_starts_with_15_darts(void) {
    TEST_ASSERT_EQ(packUnitCount(WEAPON, DART), 15,
                   "the player should start with 15 darts");
    return !testFailed;
}

static boolean player_starts_with_leather_armor_equipped(void) {
    item *armor = equippedArmor();
    TEST_ASSERT(armor != NULL, "the player should start with armor equipped");
    if (armor == NULL) {
        return !testFailed;
    }
    TEST_ASSERT_EQ(itemKind(armor), LEATHER_ARMOR,
                   "the starting armor should be leather armor");
    TEST_ASSERT(!itemIsCursed(armor),
                "the starting armor should not be cursed");
    TEST_ASSERT(!itemIsRunic(armor),
                "the starting armor should not be runic");
    return !testFailed;
}

// ============================================================
// Easy mode
// ============================================================

static void normal_damage_setup(void) {
    setPlayerHealth(playerMaxHealth());
    before = playerHealth();
    woundPlayer(15);
}

static boolean normal_mode_takes_full_damage(void) {
    TEST_ASSERT_EQ(playerHealth(), before - 15,
                   "in normal mode you should take the full damage");
    return !testFailed;
}

static void easy_mode_damage_setup(void) {
    enterEasyMode();
    setPlayerHealth(playerMaxHealth());
    before = playerHealth();
    woundPlayer(15);
}

static boolean easy_mode_reduces_damage(void) {
    TEST_ASSERT_EQ(playerHealth(), before - 3,
                   "in easy mode you should take only a fifth of the damage");
    return !testFailed;
}

// Enabling easy mode through the game's own command asks for confirmation and
// then announces the cost of the bargain.  The announcement is emitted after
// the mode changes, so in test mode it reaches the message archive rather than
// the test message log; read it from there.
static boolean easy_mode_announces_income_tax(void) {
    boolean announced = false;
    for (int back = 1; back <= messageArchiveEntryCount(); back++) {
        if (strstr(messageArchiveEntry(back), "90% income tax") != NULL) {
            announced = true;
            break;
        }
    }
    TEST_ASSERT(announced,
                "enabling easy mode should announce that the player will suffer a 90% income tax");
    return !testFailed;
}

// ============================================================
// BR73625 — if level generation fails to place the amulet altar on the
// amulet's floor, the game places an amulet somewhere on that floor as
// a fallback.
// ============================================================

// The framework rigs the altar machine to fail before descending, so the
// fallback placement is the only way an amulet can appear.
static void amulet_altar_fails_setup(void) {
    descendWithFailedAmuletAltar();
}

static boolean amulet_fallback_placed(void) {
    TEST_ASSERT_EQ(currentDepth(), 26,
                   "the amulet's floor should be depth 26");
    TEST_ASSERT(!playerIsCarryingAmulet(),
                "the player should not be carrying the amulet at this point");
    TEST_ASSERT(findFloorItem(AMULET, 0) != NULL,
                "with the altar failed, an amulet should be placed somewhere on the floor");
    return !testFailed;
}

// Negative test: when an amulet already exists (in the pack), the failed
// altar must not cause another amulet to be placed on the floor.
static void amulet_altar_fails_while_carrying_setup(void) {
    givePlayerItem(AMULET, 0, 0);
    descendWithFailedAmuletAltar();
}

static boolean no_fallback_amulet_when_already_carrying(void) {
    TEST_ASSERT_EQ(currentDepth(), 26,
                   "the amulet's floor should be depth 26");
    TEST_ASSERT_EQ(packUnitCount(AMULET, 0), 1,
                   "the player should still carry the one amulet");
    TEST_ASSERT(findFloorItem(AMULET, 0) == NULL,
                "the fallback should not place an amulet when one already exists");
    return !testFailed;
}

// ============================================================
// BR87073 — a victory saves a high score only when not in wizard
// mode and not during playback.
// ============================================================

static int victoryGold;
static int expectedScore;

// Shared setup: at depth 1, carrying the amulet, with up stairs ready to
// step onto.  Stepping onto them ends the game with an escape victory.  Each
// test picks a distinctive purse, so the score its victory would record is
// unique and can be told apart from every other entry in the file.
static void victory_common_setup(void) {
    clearArena();
    givePlayerItem(AMULET, 0, 0);
    givePlayerGold(victoryGold);
    placeUpStairs(posOffset(getPlayerPos(), 0, -1));
    expectedScore = expectedVictoryScore();
}

static void normal_victory_setup(void) {
    victoryGold = 8888888;
    victory_common_setup();
}

static void wizard_victory_setup(void) {
    victoryGold = 7777777;
    victory_common_setup();
    enterWizardMode();
    // Wizard mode turns off the test-mode input plumbing, so the victory's
    // prompts must never run: server mode drops the save-recording prompt and
    // the score screen, and autopilot skips the --MORE-- waits.
    enableServerMode();
    enableAutopilot();
}

static void playback_victory_setup(void) {
    victoryGold = 6666666;
    victory_common_setup();
    enterPlaybackMode();
}

static boolean escape_the_dungeon_step(void) {
    moveUp();
    return false;
}

static boolean normal_victory_saves_score(void) {
    TEST_ASSERT_EQ(currentDepth(), 0,
                   "stepping onto the up stairs with the amulet should escape the dungeon");
    TEST_ASSERT(highScoreHasScore(expectedScore),
                "a normal-mode victory should save its score to the high-score list");
    return !testFailed;
}

static boolean wizard_victory_saves_no_score(void) {
    TEST_ASSERT_EQ(currentDepth(), 0,
                   "stepping onto the up stairs with the amulet should escape the dungeon");
    TEST_ASSERT(!highScoreHasScore(expectedScore),
                "a wizard-mode victory must not be saved to the high-score list");
    return !testFailed;
}

static boolean playback_victory_saves_no_score(void) {
    TEST_ASSERT_EQ(currentDepth(), 0,
                   "stepping onto the up stairs with the amulet should escape the dungeon");
    TEST_ASSERT(!highScoreHasScore(expectedScore),
                "a victory during playback must not be saved to the high-score list");
    return !testFailed;
}

// ============================================================
// BR22777 — wizard item creation refuses to start when the pack is full
// ============================================================

static int br22777_pack_before;

static void br22777_full_pack_setup(void) {
    fillPackToCapacity();
    br22777_pack_before = packItemCount();
}

static boolean br22777_full_pack_step(void) {
    // "a" picks "Item" in the wizard's "Create:" dialog; with the pack full
    // the creation flow must refuse before any further prompt appears.
    queueKeystroke('a', false, false);
    attemptWizardItemCreation();
    return false;
}

static boolean br22777_creation_refused_when_pack_full(void) {
    TEST_ASSERT_EQ(br22777_pack_before, 26,
                   "the pack should be at its capacity before the attempt");
    TEST_ASSERT_MSG(messageWasShown("Your pack is already full."),
                    "wizard item creation should refuse a full pack");
    TEST_ASSERT_EQ(packItemCount(), br22777_pack_before,
                   "a refused creation must add nothing to the pack");
    TEST_ASSERT(!messageWasShown("you now have"),
                "a refused creation must not report a created item");
    return !testFailed;
}

// ============================================================
// BR64450 — a wizard-created item can be identified immediately on request
// ============================================================

// Walk the wizard's creation dialogs from queued keystrokes: "Item" (a),
// the "potion" category (d), its first kind (a), and finally the answer to
// the "Identify the item?" prompt the creation flow ends with.
static void br64450_create_potion(char identifyAnswer) {
    queueKeystroke('a', false, false);
    queueKeystroke('d', false, false);
    queueKeystroke('a', false, false);
    queueKeystroke(identifyAnswer, false, false);
    attemptWizardItemCreation();
}

static void br64450_accept_identify_setup(void) {
    br64450_create_potion('y');
}

static boolean br64450_created_item_is_identified(void) {
    item *created = findInventoryItem(POTION, 0);
    TEST_ASSERT(created != NULL, "the wizard should have created a potion");
    TEST_ASSERT(messageWasShown("you now have"),
                "creation should report the item you now have");
    TEST_ASSERT(itemIsIdentified(created),
                "answering yes should identify the created item at once");
    return !testFailed;
}

static void br64450_decline_identify_setup(void) {
    br64450_create_potion('n');
}

static boolean br64450_created_item_stays_unknown(void) {
    item *created = findInventoryItem(POTION, 0);
    TEST_ASSERT(created != NULL, "the wizard should have created a potion");
    TEST_ASSERT(!itemIsIdentified(created),
                "declining the request should leave the created item unidentified");
    return !testFailed;
}

// ============================================================
// BR88672 — an item created in wizard mode is not cursed unless
// its enchantment is negative.
//
// The creation dialog is driven from queued keystrokes: "Item" (a),
// the "ring" category (h), its first kind (a), the enchantment
// number, Return to accept it, and yes to identify the item.  A ring
// is used because its creation flow asks for an enchantment and
// allows a negative value.  A created +1 ring must not be cursed,
// and a created -3 ring must be.
// ============================================================

static void br88672_create_ring(const char *enchantment) {
    queueKeystroke('a', false, false); // Item in the Create: dialog
    queueKeystroke('h', false, false); // ring category
    queueKeystroke('a', false, false); // first ring kind
    for (const char *c = enchantment; *c != '\0'; c++) {
        queueKeystroke(*c, false, false);
    }
    queueKeystroke(RETURN_KEY, false, false); // accept the enchantment
    queueKeystroke('y', false, false); // identify the created ring
    attemptWizardItemCreation();
}

static void br88672_positive_enchant_setup(void) {
    br88672_create_ring("1");
}

static boolean br88672_positive_enchant_not_cursed(void) {
    item *created = findInventoryItem(RING, 0);
    TEST_ASSERT(created != NULL, "the wizard should have created a ring");
    if (created != NULL) {
        TEST_ASSERT_EQ(itemEnchant(created), 1,
                       "the created ring should keep the enchantment that was entered");
        TEST_ASSERT(!itemIsCursed(created),
                    "a created ring with non-negative enchantment should not be cursed");
    }
    return !testFailed;
}

static void br88672_negative_enchant_setup(void) {
    br88672_create_ring("-3");
}

static boolean br88672_negative_enchant_cursed(void) {
    item *created = findInventoryItem(RING, 0);
    TEST_ASSERT(created != NULL, "the wizard should have created a ring");
    if (created != NULL) {
        TEST_ASSERT_EQ(itemEnchant(created), -3,
                       "the created ring should keep the enchantment that was entered");
        TEST_ASSERT(itemIsCursed(created),
                    "a created ring with negative enchantment should be cursed");
    }
    return !testFailed;
}

// ============================================================
// BR24559 — a wizard-created monster can only be placed on a location
// the player can see or otherwise sense.
// BR11100 — a wizard-created turret can only be placed on a wall.
// BR13174 — a wizard-created monster that is not a turret can only
// be placed on an unobstructed location.
// BR15672 — choosing an invalid location discards the monster.
//
// The creation dialog is driven from queued input: "Monster" (b),
// the first page of monster names (a), then a monster on that page.
// Two clicks on a dungeon tile move the targeting cursor there and
// confirm it.  Goblin totem is inanimate, so the flow asks for no
// mutation; arrow turret is a turret by its display character.  A
// valid placement reports "<monster> created." and adds the monster
// to the level; an invalid one reports "Invalid location." and
// leaves the level's monster count unchanged.
// ============================================================

static pos br24559_wall, br24559_openFloor, br24559_unseenFloor;
static int br24559_monstersBefore;

static void br24559_setup(void) {
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(3, FLOOR);

    // A visible wall, a visible open spot beside it, and an open spot far
    // outside the light.
    br24559_wall = posOffset(getPlayerPos(), 3, 0);
    setTerrain(br24559_wall, WALL);
    br24559_openFloor = posOffset(getPlayerPos(), 0, 2);
    setTerrain(br24559_openFloor, FLOOR);
    // Seal the far spot inside a one-tile chamber so no level layout can put
    // it in sight.
    br24559_unseenFloor = posOffset(getPlayerPos(), 20, 0);
    setTerrainAroundLoc(br24559_unseenFloor, WALL);
    setTerrain(br24559_unseenFloor, FLOOR);

    // Recompute vision and scent over the newly carved arena, then mark the
    // two spots the test treats as visible.
    restOneTurn();
    revealCell(br24559_wall);
    revealCell(br24559_openFloor);

    br24559_monstersBefore = monsterCount();
}

// Walk the wizard's monster creation dialogs from queued input and aim the
// creation cursor at `loc` (two clicks: move, then confirm).
static void br24559_create_monster(int monsterKey, pos loc) {
    queueKeystroke('b', false, false); // "Monster" in the Create: dialog
    queueKeystroke('a', false, false); // first page of monster names
    queueKeystroke(monsterKey, false, false);
    queueMapClick(loc);
    queueMapClick(loc);
    attemptWizardItemCreation();
}

static void br24559_open_ground_setup(void) {
    br24559_setup();
    br24559_create_monster('v', br24559_openFloor); // goblin totem
}

static boolean br24559_monster_placed_on_open_ground(void) {
    creature *placed = findMonster(MK_GOBLIN_TOTEM);
    TEST_ASSERT(cellIsVisible(br24559_openFloor),
                "control: the chosen open ground should be visible");
    TEST_ASSERT(placed != NULL,
                "a monster placed on visible open ground should exist");
    if (placed != NULL) {
        TEST_ASSERT_POS_EQ(monsterPosition(placed), br24559_openFloor,
                           "the monster should stand on the chosen tile");
    }
    TEST_ASSERT(messageWasShown("goblin totem created."),
                "a valid placement should report the monster as created");
    TEST_ASSERT_EQ(monsterCount(), br24559_monstersBefore + 1,
                   "a valid placement should add exactly one monster");
    return !testFailed;
}

static void br24559_wall_for_monster_setup(void) {
    br24559_setup();
    br24559_create_monster('v', br24559_wall); // non-turret on a wall
}

static boolean br24559_monster_rejected_on_wall(void) {
    TEST_ASSERT(cellIsVisible(br24559_wall),
                "control: the chosen wall should be visible");
    TEST_ASSERT(messageWasShown("Invalid location."),
                "a wall should be invalid ground for an ordinary monster");
    TEST_ASSERT(!messageWasShown("goblin totem created."),
                "an invalid choice must not report the monster as created");
    TEST_ASSERT_EQ(monsterCount(), br24559_monstersBefore,
                   "an invalid choice should discard the created monster");
    return !testFailed;
}

static void br24559_unseen_ground_setup(void) {
    br24559_setup();
    br24559_create_monster('v', br24559_unseenFloor);
}

static boolean br24559_monster_rejected_where_unseen(void) {
    TEST_ASSERT(!cellIsVisible(br24559_unseenFloor),
                "control: the chosen open ground should lie out of sight");
    TEST_ASSERT(messageWasShown("Invalid location."),
                "a location you cannot see should be invalid");
    TEST_ASSERT_EQ(monsterCount(), br24559_monstersBefore,
                   "an unseen choice should discard the created monster");
    return !testFailed;
}

static void br24559_turret_on_wall_setup(void) {
    br24559_setup();
    br24559_create_monster('c', br24559_wall); // arrow turret
}

static boolean br24559_turret_placed_on_wall(void) {
    creature *placed = findMonster(MK_ARROW_TURRET);
    TEST_ASSERT(cellIsTerrain(br24559_wall, WALL),
                "control: the chosen tile should still be a wall");
    TEST_ASSERT(placed != NULL,
                "a turret placed on a visible wall should exist");
    if (placed != NULL) {
        TEST_ASSERT_POS_EQ(monsterPosition(placed), br24559_wall,
                           "the turret should stand in the chosen wall");
    }
    TEST_ASSERT(messageWasShown("arrow turret created."),
                "a valid turret placement should report the turret as created");
    TEST_ASSERT_EQ(monsterCount(), br24559_monstersBefore + 1,
                   "a valid turret placement should add exactly one monster");
    return !testFailed;
}

static void br24559_turret_on_open_ground_setup(void) {
    br24559_setup();
    br24559_create_monster('c', br24559_openFloor);
}

static boolean br24559_turret_rejected_on_open_ground(void) {
    TEST_ASSERT(cellIsVisible(br24559_openFloor),
                "control: the chosen open ground should be visible");
    TEST_ASSERT(messageWasShown("Invalid location."),
                "open ground should be invalid for a turret");
    TEST_ASSERT_EQ(monsterCount(), br24559_monstersBefore,
                   "an invalid turret choice should discard the turret");
    return !testFailed;
}

// ============================================================
// BR10231 — the monster-creation placement prompt asks for the kind of
// square the created monster needs: a visible wall for a turret, a
// visible unobstructed location for any other monster
// ============================================================

static void br10231_turret_prompt_setup(void) {
    br24559_setup();
    br24559_create_monster('c', br24559_wall); // arrow turret
}

static boolean br10231_turret_prompt_asks_for_wall(void) {
    TEST_ASSERT(messageWasShown("Create arrow turret where? Choose a visible wall."),
                "the turret placement prompt should ask for a visible wall");
    TEST_ASSERT(!messageWasShown("unobstructed location"),
                "the turret placement prompt should not ask for an unobstructed location");
    return !testFailed;
}

static void br10231_monster_prompt_setup(void) {
    br24559_setup();
    br24559_create_monster('v', br24559_openFloor); // goblin totem
}

static boolean br10231_monster_prompt_asks_for_open_location(void) {
    TEST_ASSERT(messageWasShown("Create goblin totem where? Choose a visible unobstructed location."),
                "an ordinary monster's placement prompt should ask for a visible unobstructed location");
    TEST_ASSERT(!messageWasShown("Choose a visible wall."),
                "an ordinary monster's placement prompt should not ask for a wall");
    return !testFailed;
}

// ============================================================
// BR27330 — Wizard mode allows the player to create an arbitrary
// monster from the catalog, excluding the lich and the phoenix.
//
// The catalog is alphabetical and paged 24 entries at a time, so
// each range label names the first and last entry on a page.  A
// correct catalog has 65 entries: page two then runs from guardian
// spirit to spark turret.  Including the lich or the phoenix shifts
// that boundary to sentinel or salamander, so the exact label is
// the falsifiable part.  The second page's list is also read back
// directly: it shows the phoenix egg and the phylactery but no lich
// and no plain phoenix.  A final test creates a kraken chosen from
// that page, proving the second page is reachable and its monsters
// are really created.
// ============================================================

static void br27330_range_setup(void) {
    queueKeystroke('b', false, false); // Monster in the Create: dialog
    attemptWizardItemCreation();
}

static boolean br27330_range_boundaries(void) {
    TEST_ASSERT(screenTextContains("Acid mound - Golem"),
                "the first monster page should start at acid mound and end at golem");
    TEST_ASSERT(screenTextContains("Guardian spirit - Spark turret"),
                "the second monster page should start at guardian spirit and end at "
                "spark turret, which is only true when both the lich and the phoenix "
                "have been removed from the catalog");
    TEST_ASSERT(screenTextContains("Spectral blade - Zombie"),
                "the last monster page should start at spectral blade and end at zombie");

    TEST_ASSERT(!screenTextContains("Guardian spirit - Sentinel"),
                "including either the lich or the phoenix would move the second "
                "page's last entry to sentinel");
    TEST_ASSERT(!screenTextContains("Guardian spirit - Salamander"),
                "including both the lich and the phoenix would move the second "
                "page's last entry to salamander");
    return !testFailed;
}

static void br27330_page_setup(void) {
    queueKeystroke('b', false, false); // Monster in the Create: dialog
    queueKeystroke('b', false, false); // second page of monster names
    attemptWizardItemCreation();
}

static boolean br27330_page_omits_lich_and_phoenix(void) {
    TEST_ASSERT(screenTextContains("Guardian spirit"),
                "the second monster page should be shown");
    TEST_ASSERT(screenTextContains("Kraken"),
                "the second monster page should list the monsters around the l..p range");
    TEST_ASSERT(screenTextContains("Phoenix egg"),
                "the second monster page should still offer the phoenix egg");
    TEST_ASSERT(screenTextContains("Phylactery"),
                "the second monster page should still offer the phylactery");
    TEST_ASSERT(!screenTextContains("Lich"),
                "the second monster page must not offer the lich");

    const char *phoenixLine = screenTextLineContaining("Phoenix");
    TEST_ASSERT(strstr(phoenixLine, "Phoenix egg") != NULL,
                "the only phoenix the catalog should offer is the phoenix egg");

    TEST_ASSERT(!screenTextContains("Spider"),
                "the second monster page should not show entries from the third page");
    return !testFailed;
}

static void br27330_create_setup(void) {
    br24559_setup();
    queueKeystroke('b', false, false); // Monster in the Create: dialog
    queueKeystroke('b', false, false); // second page of monster names
    queueKeystroke('f', false, false); // kraken, on that page
    queueKeystroke(ESCAPE_KEY, false, false); // decline the mutation offer
    queueMapClick(br24559_openFloor); // move the target cursor
    queueMapClick(br24559_openFloor); // confirm the target
    queueKeystroke(ESCAPE_KEY, false, false); // do not make it an ally
    attemptWizardItemCreation();
}

static boolean br27330_monster_created_from_second_page(void) {
    creature *kraken = findMonster(MK_KRAKEN);
    TEST_ASSERT(kraken != NULL,
                "a monster chosen from the catalog's second page should be created");
    if (kraken != NULL) {
        TEST_ASSERT_POS_EQ(monsterPosition(kraken), br24559_openFloor,
                           "the kraken should stand on the chosen visible floor tile");
    }
    TEST_ASSERT(messageWasShown("kraken created."),
                "a valid placement should report the kraken as created");
    TEST_ASSERT_EQ(monsterCount(), br24559_monstersBefore + 1,
                   "creating the kraken should add exactly one monster");
    return !testFailed;
}

// ============================================================
// BR78012 — the welcome message tells the player to retrieve the
// Amulet of Yendor from a specific floor of the dungeon and escape
// with it, naming that floor with an ordinal number
// ============================================================

static boolean welcome_message_names_amulet_floor(void) {
    const char *msg = startupMessageWith("Retrieve the");
    TEST_ASSERT(msg != NULL,
                "the game should greet the player with a message about retrieving something");
    if (msg == NULL) {
        return !testFailed;
    }
    TEST_ASSERT(strstr(msg, "Amulet of Yendor") != NULL,
                "the welcome message should name the Amulet of Yendor");
    TEST_ASSERT(strstr(msg, "escape with it") != NULL,
                "the welcome message should tell the player to escape with it");

    const char *floorText = strstr(msg, "from the ");
    TEST_ASSERT(floorText != NULL,
                "the welcome message should say which floor holds the amulet");
    if (floorText == NULL) {
        return !testFailed;
    }

    int floorNumber = 0;
    char suffix[4] = "";
    int fields = sscanf(floorText, "from the %d%3s floor", &floorNumber, suffix);
    TEST_ASSERT(fields == 2,
                "the welcome message should read \"from the <number> floor\"");
    if (fields != 2) {
        return !testFailed;
    }

    TEST_ASSERT(floorNumber >= 1,
                "the welcome message should name a real dungeon floor");
    int lastTwoDigits = floorNumber % 100;
    int lastDigit = floorNumber % 10;
    const char *expectedSuffix;
    if (lastTwoDigits >= 11 && lastTwoDigits <= 13) {
        expectedSuffix = "th";
    } else if (lastDigit == 1) {
        expectedSuffix = "st";
    } else if (lastDigit == 2) {
        expectedSuffix = "nd";
    } else if (lastDigit == 3) {
        expectedSuffix = "rd";
    } else {
        expectedSuffix = "th";
    }
    TEST_ASSERT(strcmp(suffix, expectedSuffix) == 0,
                "the amulet floor should be named with its correct ordinal number");
    return !testFailed;
}

// ============================================================
// BR99222 — A victory screen tallies the carried gold plus the value of
// every carried item to produce a total treasure value, and the amulet's
// value is doubled on the greater victory.
// ============================================================

// The victory screen's total is the score the run records, so both tests
// start from an empty high-score list (clearHighScores) and assert on the
// exact score saved: the tally itself, an under-count, and gold alone must
// all be told apart.  Each setup takes a distinctive purse so its total is
// unique across the whole file.  A lumenstone rides along in the pack so
// "every carried item" covers more than just the amulet.

static int br99222_gold;
static int br99222_total;      // the tally this victory should record
static int br99222_amuletValue;
static int br99222_gemValue;

// Escape victory: stepping onto the up stairs with the amulet, as in the
// BR87073 tests above.
static void br99222_escape_setup(void) {
    br99222_gold = 1234567;
    clearHighScores();
    clearArena();
    givePlayerItem(AMULET, 0, 0);
    givePlayerItem(GEM, 0, 0);
    givePlayerGold(br99222_gold);
    placeUpStairs(posOffset(getPlayerPos(), 0, -1));
    br99222_gemValue = (int) itemGoldValue(findInventoryItem(GEM, 0));
    br99222_total = expectedVictoryScore();
}

static boolean br99222_escape_records_full_treasure(void) {
    TEST_ASSERT_EQ(currentDepth(), 0,
                   "stepping onto the up stairs with the amulet should escape the dungeon");
    TEST_ASSERT(messageWasShown("You are bathed in sunlight"),
                "the escape victory should have run");
    TEST_ASSERT(highScoreHasScore(br99222_total),
                "the recorded total should be carried gold plus the value of every carried item");
    TEST_ASSERT(!highScoreHasScore(br99222_gold),
                "gold alone must not be the total: item values belong in it too");
    TEST_ASSERT(!highScoreHasScore(br99222_total - br99222_gemValue),
                "every carried item counts: dropping the lumenstone's value must not match");
    return !testFailed;
}

// Greater victory: from the deepest floor, descending while carrying the
// amulet transcends instead of escaping.
static void br99222_greater_setup(void) {
    br99222_gold = 7654321;
    clearHighScores();
    clearArena();
    givePlayerItem(AMULET, 0, 0);
    givePlayerItem(GEM, 0, 0);
    givePlayerGold(br99222_gold);
    setDepth(40); // the dungeon's deepest floor
    placeDownStairs(posOffset(getPlayerPos(), 0, 1));
    br99222_amuletValue = (int) itemGoldValue(findInventoryItem(AMULET, 0));
    // The same tally as an escape, plus the amulet counted a second time.
    br99222_total = expectedVictoryScore() + br99222_amuletValue;
}

static boolean br99222_greater_victory_step(void) {
    moveDown();
    return false;
}

static boolean br99222_greater_doubles_amulet_value(void) {
    TEST_ASSERT(messageWasShown("Light streams through the portal"),
                "descending with the amulet from the deepest floor should transcend");
    TEST_ASSERT(highScoreHasScore(br99222_total),
                "the greater victory's total should count the amulet's value twice");
    TEST_ASSERT(!highScoreHasScore(br99222_total - br99222_amuletValue),
                "counting the amulet once would be the escape tally, not the greater victory's");
    return !testFailed;
}

// ============================================================
// BR69169 — before the welcome message, the message log and archive
// are cleared and the screen is blacked out.
//
// Something from "before" the game starts is planted first: an archived
// message lands in the message archive and is drawn in the message lines
// at the edge of the screen (both are asserted as preconditions).  The
// game is then started over with the same seed, which reruns the whole
// start-up sequence, and nothing of the planted message may survive it.
// ============================================================

// Capitalised because the archive capitalises whatever it renders, which
// keeps this exact text searchable with strstr().
static const char br69169_marker[] = "Leftover message from an earlier game";

static void br69169_setup(void) {
    emitArchivedMessage(br69169_marker);
    TEST_ASSERT(strstr(renderedMessageLine(), br69169_marker) != NULL,
                "setup: the marker should be in the message archive");
    runQueuedInputThroughMainLoop(); // capture a frame of the current screen
    TEST_ASSERT(screenTextContains(br69169_marker),
                "setup: the marker should be drawn in the message lines");

    // Start the game over: the log and archive are cleared, the screen is
    // blacked out, and the player is welcomed into a fresh dungeon.
    restartGame();
    runQueuedInputThroughMainLoop(); // capture a frame of the new game's screen
}

static boolean start_clears_messages_and_blacks_out_screen(void) {
    TEST_ASSERT(messageWasShown("Hello and welcome"),
                "the fresh start should welcome the player");
    TEST_ASSERT(strstr(renderedMessageLine(), br69169_marker) == NULL,
                "the message archive should hold nothing from before the welcome message");
    TEST_ASSERT(!screenTextContains(br69169_marker),
                "the message log should be cleared and the screen blacked out "
                "before the welcome message");
    return !testFailed;
}

// ============================================================
// BR44882 — when the player dies and the run is not a playback, the
// score is saved to the high-score list and the high scores are
// displayed.
//
// Starving at one health ends the run through the ordinary death
// path.  The normal case must both write the score and open the
// high-score display; the playback case must do neither.  The display
// is recorded by the engine when printHighScores() opens it, because
// the save-recording prompt that follows a death covers the screen
// before the test's last-input snapshot is taken.
// ============================================================

#define BR44882_SCORE 44882

static void br44882_common_setup(void) {
    clearArena();
    clearMonsters();
    clearHighScores();
    givePlayerGold(BR44882_SCORE);
    setPlayerHealth(1);
    setPlayerNutrition(0);
}

static void br44882_death_setup(void) {
    br44882_common_setup();
}

static void br44882_playback_death_setup(void) {
    br44882_common_setup();
    enterPlaybackMode();
}

static boolean br44882_starve_step(void) {
    restOneTurn();
    return false;
}

static boolean br44882_death_saves_and_displays_scores(void) {
    TEST_ASSERT(isPlayerDead(), "the player should have starved to death");
    TEST_ASSERT(runHasEnded(), "the death should end the run");
    TEST_ASSERT(highScoreHasScore(BR44882_SCORE),
                "a death outside playback should save its score");
    TEST_ASSERT(highScoresWereDisplayed(),
                "a death outside playback should display the high scores");
    return !testFailed;
}

static boolean br44882_playback_death_is_not_scored(void) {
    TEST_ASSERT(isPlayerDead(), "the player should have starved to death");
    TEST_ASSERT(runHasEnded(), "the death should end the run");
    TEST_ASSERT(!highScoreHasScore(BR44882_SCORE),
                "a death during playback must not save a score");
    TEST_ASSERT(!highScoresWereDisplayed(),
                "a death during playback must not display the high scores");
    return !testFailed;
}

// ============================================================
// BR74433 — the high-score entry saved on death describes only the
// killer and depth, omitting the treasure amount.
// ============================================================

static int br74433_score;

static void br74433_setup(void) {
    clearArena();
    clearHighScores();
    br74433_score = 74433;
    givePlayerGold(br74433_score);
    setPlayerHealth(1);
    setPlayerNutrition(0);
}

static boolean br74433_starve_step(void) {
    restOneTurn();
    return false;
}

static boolean br74433_high_score_omits_treasure(void) {
    TEST_ASSERT(isPlayerDead(),
                "the player should have starved to death");
    TEST_ASSERT(highScoreHasScore(br74433_score),
                "the death should save the player's score");
    const char *description = highScoreDescription(br74433_score);
    TEST_ASSERT(strcmp(description, "Starved to death on depth 1.") == 0,
                "the saved entry should name only the killer and depth");
    TEST_ASSERT(strstr(description, "gold") == NULL,
                "the saved entry must omit the treasure amount");
    return !testFailed;
}

// ============================================================
// BR90833 — the death message states the killer and depth as
// "Killed by a/an X on depth N", or a custom phrase followed by
// "on depth N".
//
// The high-score entry stores exactly that sentence (without the
// treasure suffix), so it exposes the death message without
// reading engine state.  A non-1 depth distinguishes a real depth
// from a hardcoded one, and a vowel-initial killer distinguishes
// the "a"/"an" article rule.
// ============================================================

#define BR90833_SCORE 90833

static void br90833_death_setup(void) {
    clearMonsters();
    clearArena();
    clearHighScores();
    givePlayerGold(BR90833_SCORE);
    setPlayerHealth(1);
}

static void br90833_troll_setup(void) {
    br90833_death_setup();
    setDepth(2);
    applyStatusToPlayer(STATUS_PARALYZED, 100);
    placeHostileMonster(MK_TROLL, posOffset(getPlayerPos(), 1, 0));
}

static void br90833_ogre_setup(void) {
    br90833_death_setup();
    setDepth(4);
    applyStatusToPlayer(STATUS_PARALYZED, 100);
    placeHostileMonster(MK_OGRE, posOffset(getPlayerPos(), 1, 0));
}

static void br90833_starve_setup(void) {
    br90833_death_setup();
    setDepth(3);
    setPlayerNutrition(0);
}

static boolean br90833_kill_step(void) {
    if (isPlayerDead() || playerTurnCount() > 50) {
        return false;
    }
    restOneTurn();
    return true;
}

static boolean br90833_death_message(const char *expected) {
    TEST_ASSERT(isPlayerDead(), "the player should have been killed");
    TEST_ASSERT(highScoreHasScore(BR90833_SCORE),
                "the death should save the player's score");
    TEST_ASSERT(strcmp(highScoreDescription(BR90833_SCORE), expected) == 0,
                "the death message should name the killer and depth");
    return !testFailed;
}

static boolean br90833_monster_death(void) {
    return br90833_death_message("Killed by a troll on depth 2.");
}

static boolean br90833_monster_vowel_death(void) {
    return br90833_death_message("Killed by an ogre on depth 4.");
}

static boolean br90833_custom_phrase_death(void) {
    return br90833_death_message("Starved to death on depth 3.");
}

// ============================================================
// BR36967 — when the player dies, the run ends.
//
// A starved player at one health dies during a single rest.  The
// check distinguishes a real ending from merely zero health: the
// run-ended state is only set once the death has been processed,
// so isPlayerDead() alone (which is also true at zero health)
// would not prove the run ended.
// ============================================================

static void br36967_setup(void) {
    clearMonsters();
    clearArena();
    setPlayerHealth(1);
    setPlayerNutrition(0);
}

static boolean br36967_starve_step(void) {
    restOneTurn();
    return false;
}

static boolean br36967_death_ends_run(void) {
    TEST_ASSERT(isPlayerDead(),
                "the player should have starved to death");
    TEST_ASSERT(runHasEnded(),
                "the player's death should end the run");
    return !testFailed;
}

// ============================================================

// ============================================================
// BR11461 — monsters near the player that are able to use
// stairs follow it to the next level, approaching the stairs
// after a delay based on their distance.
// ============================================================

static creature *br11461_near;
static creature *br11461_far;
static creature *br11461_unable;
static int br11461_nearArrivalTurn;
static int br11461_farArrivalTurn;
static int br11461_turns;
static int br11461_oldDepth;

static void br11461_setup(void) {
    // Open a clear stretch of floor around the player so both followers have
    // an unobstructed route to the stairs, and space them so the near and far
    // monsters differ only in distance (both are rats, with equal speed).
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(12, FLOOR);

    br11461_near = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 3, 0));
    br11461_far = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 9, 0));
    // Control: a monster that cannot use stairs must stay behind, proving
    // the test observes stair use rather than every monster being moved.
    br11461_unable = placeHostileMonster(MK_GOBLIN_TOTEM, posOffset(getPlayerPos(), 6, 0));

    br11461_oldDepth = currentDepth();
    br11461_nearArrivalTurn = -1;
    br11461_farArrivalTurn = -1;
    br11461_turns = 0;
    descendToNextLevel();
}

static boolean br11461_step(void) {
    if (br11461_nearArrivalTurn < 0
        && monsterDepth(br11461_near) == currentDepth()) {
        br11461_nearArrivalTurn = br11461_turns;
    }
    if (br11461_farArrivalTurn < 0
        && monsterDepth(br11461_far) == currentDepth()) {
        br11461_farArrivalTurn = br11461_turns;
    }
    if (br11461_nearArrivalTurn >= 0 && br11461_farArrivalTurn >= 0) {
        return false;
    }
    if (br11461_turns >= 60) {
        return false;
    }
    br11461_turns++;
    restOneTurn();
    return true;
}

static boolean br11461_near_monster_arrives_first(void) {
    TEST_ASSERT_EQ(currentDepth(), br11461_oldDepth + 1,
                   "the player should have descended one level");
    TEST_ASSERT(br11461_nearArrivalTurn >= 0,
                "the near monster should follow the player to the next level");
    TEST_ASSERT(br11461_farArrivalTurn >= 0,
                "the far monster should follow the player to the next level");
    TEST_ASSERT(br11461_nearArrivalTurn < br11461_farArrivalTurn,
                "the nearer monster should arrive at the stairs before the farther one");
    TEST_ASSERT_EQ(monsterDepth(br11461_unable), br11461_oldDepth,
                   "a monster that cannot use stairs should stay on the old level");
    return !testFailed;
}

const testCase tests_progression[] = {
    { "BR19079_new_game_begins_on_depth_1",        NULL,                    NULL, new_game_begins_on_depth_1 },
    { "BR89567_new_game_starts_with_no_gold",      NULL,                    NULL, new_game_starts_with_no_gold },
    { "BR59025_new_game_starts_at_turn_zero",      NULL,                    NULL, new_game_starts_at_turn_zero },
    { "BR74104_player_starts_with_12_strength",    NULL,                    NULL, player_starts_with_12_strength },
    { "BR84421_player_starts_with_food_ration",    NULL,                    NULL, player_starts_with_food_ration },
    { "BR49147_player_starts_with_dagger_equipped", NULL,                   NULL, player_starts_with_dagger_equipped },
    { "BR81759_player_starts_with_15_darts",       NULL,                    NULL, player_starts_with_15_darts },
    { "BR27339_player_starts_with_leather_armor_equipped", NULL,            NULL, player_starts_with_leather_armor_equipped },

    { "BR88010_normal_mode_takes_full_damage",     normal_damage_setup,     NULL, normal_mode_takes_full_damage },
    { "BR88010_easy_mode_reduces_damage",          easy_mode_damage_setup,  NULL, easy_mode_reduces_damage },
    { "BR90722_easy_mode_announces_income_tax",    enableEasyModeThroughCommand, NULL, easy_mode_announces_income_tax },

    { "BR73625_failed_altar_fallback_places_amulet", amulet_altar_fails_setup, NULL, amulet_fallback_placed },
    { "BR73625_failed_altar_spares_a_carried_amulet", amulet_altar_fails_while_carrying_setup, NULL, no_fallback_amulet_when_already_carrying },

    { "BR87073_normal_victory_saves_high_score",  normal_victory_setup,  escape_the_dungeon_step, normal_victory_saves_score },
    { "BR87073_wizard_victory_saves_no_score",    wizard_victory_setup,  escape_the_dungeon_step, wizard_victory_saves_no_score },
    { "BR87073_playback_victory_saves_no_score",  playback_victory_setup, escape_the_dungeon_step, playback_victory_saves_no_score },

    { "BR22777_creation_refused_when_pack_full",  br22777_full_pack_setup, br22777_full_pack_step, br22777_creation_refused_when_pack_full },

    { "BR64450_created_item_identified_on_request", br64450_accept_identify_setup, NULL, br64450_created_item_is_identified },
    { "BR64450_created_item_unidentified_when_declined", br64450_decline_identify_setup, NULL, br64450_created_item_stays_unknown },

    { "BR88672_created_positive_enchant_item_not_cursed", br88672_positive_enchant_setup, NULL, br88672_positive_enchant_not_cursed },
    { "BR88672_created_negative_enchant_item_cursed", br88672_negative_enchant_setup, NULL, br88672_negative_enchant_cursed },

    { "BR24559_monster_placed_on_open_ground", br24559_open_ground_setup, NULL, br24559_monster_placed_on_open_ground },
    { "BR24559_monster_rejected_on_wall", br24559_wall_for_monster_setup, NULL, br24559_monster_rejected_on_wall },
    { "BR24559_monster_rejected_where_unseen", br24559_unseen_ground_setup, NULL, br24559_monster_rejected_where_unseen },
    { "BR24559_turret_placed_on_wall", br24559_turret_on_wall_setup, NULL, br24559_turret_placed_on_wall },
    { "BR24559_turret_rejected_on_open_ground", br24559_turret_on_open_ground_setup, NULL, br24559_turret_rejected_on_open_ground },

    { "BR10231_turret_prompt_asks_for_wall", br10231_turret_prompt_setup, NULL, br10231_turret_prompt_asks_for_wall },
    { "BR10231_monster_prompt_asks_for_open_location", br10231_monster_prompt_setup, NULL, br10231_monster_prompt_asks_for_open_location },

    { "BR27330_monster_page_ranges_exclude_lich_and_phoenix", br27330_range_setup, NULL, br27330_range_boundaries },
    { "BR27330_second_monster_page_omits_lich_and_phoenix", br27330_page_setup, NULL, br27330_page_omits_lich_and_phoenix },
    { "BR27330_monster_created_from_second_page", br27330_create_setup, NULL, br27330_monster_created_from_second_page },

    { "BR78012_welcome_message_names_amulet_floor", NULL, NULL, welcome_message_names_amulet_floor },
    { "BR69169_start_clears_messages_and_blacks_screen", br69169_setup, NULL, start_clears_messages_and_blacks_out_screen },

    { "BR99222_escape_records_full_treasure",  br99222_escape_setup, escape_the_dungeon_step, br99222_escape_records_full_treasure },
    { "BR99222_greater_doubles_amulet_value",  br99222_greater_setup, br99222_greater_victory_step, br99222_greater_doubles_amulet_value },

    { "BR44882_death_saves_and_displays_high_scores", br44882_death_setup, br44882_starve_step, br44882_death_saves_and_displays_scores },
    { "BR44882_playback_death_is_not_scored",         br44882_playback_death_setup, br44882_starve_step, br44882_playback_death_is_not_scored },

    { "BR74433_death_high_score_omits_treasure", br74433_setup, br74433_starve_step, br74433_high_score_omits_treasure },

    { "BR90833_death_names_killer_and_depth", br90833_troll_setup, br90833_kill_step, br90833_monster_death },
    { "BR90833_death_uses_an_before_vowel", br90833_ogre_setup, br90833_kill_step, br90833_monster_vowel_death },
    { "BR90833_custom_phrase_names_depth", br90833_starve_setup, br90833_kill_step, br90833_custom_phrase_death },

    { "BR36967_death_ends_run", br36967_setup, br36967_starve_step, br36967_death_ends_run },

    { "BR11461_near_monster_arrives_first",
      br11461_setup, br11461_step, br11461_near_monster_arrives_first },

    { NULL, NULL, NULL, NULL }
};
