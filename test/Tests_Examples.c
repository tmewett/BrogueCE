#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

const short TEMP_STATUS_AMOUNT = 5;

typedef void (*setupFn)(void);


// ------- SETUP FUNCTIONS -------

static void floor_setup(void) {
    setTerrainAroundLoc(player.loc, FLOOR);
    setPlayerPos(player.loc);
}

static void chasm_setup(void) {
    // Create a chasm around/under the player
    setTerrainAroundLoc(player.loc, CHASM);
    setPlayerPos(player.loc);   // Replace player
}

static void bog_setup(void) {
    setTerrainAroundLoc(player.loc, MUD);
    setPlayerPos(player.loc); // Replace player on tile
}

static void water_setup(void) {
    setTerrainAroundLoc(player.loc, DEEP_WATER);
    setPlayerPos(player.loc); // Replace player on tile

    // Give player items that will float away
    givePlayerItem(WEAPON,SPEAR,0);
    givePlayerItem(WEAPON,MACE,0);
    givePlayerItem(WEAPON,WHIP,0);
    givePlayerItem(WEAPON,AXE,0);
    givePlayerItem(WEAPON,PIKE,0);
}

static void trap_setup(void) {
    floor_setup();

    // Place pressure plate trap next to player
    pos trapLoc = {player.loc.x + 1, player.loc.y};
    setTerrain(trapLoc, MACHINE_PRESSURE_PLATE);
}

static void gas_setup(void) {
    pos gasLoc = player.loc;
    spawnFeature(gasLoc, DF_POISON_GAS_CLOUD);
}

static void bog_monster_setup(void) {
    bog_setup();
    clearMonsters();

    // Spawn a bog monster (MA_SEIZES) adjacent
    pos bogLoc = {player.loc.x + 1, player.loc.y};
    creature *bog = generateMonster(MK_BOG_MONSTER, false, false);
    setMonsterLocation(bog, bogLoc);
    makeMonsterHostile(bog);
}

static void rat_setup(void) {
    floor_setup();
    clearMonsters();

    // Spawn a rat adjacent
    pos ratLoc = {player.loc.x + 1, player.loc.y};
    creature *rat = generateMonster(MK_RAT, false, false);
    setMonsterLocation(rat, ratLoc);
    makeMonsterHostile(rat);
}

static void eel_setup(void) {
    water_setup();
    clearMonsters();

    pos eelLoc = {player.loc.x + 1, player.loc.y};
    creature *eel = generateMonster(MK_EEL, false, false);
    setMonsterLocation(eel, eelLoc);
    makeMonsterHostile(eel);

    drink(POTION_LIFE); // Give the player enough HP to survive for 10 turns against eel
}

static void bloat_setup(void) {
    chasm_setup();
    clearMonsters();

    // Spawn a bloat (has MONST_FLIES)
    pos bloatLoc = {player.loc.x + 1, player.loc.y};
    creature *bloat = generateMonster(MK_BLOAT, false, false);
    setMonsterLocation(bloat, bloatLoc);
    makeMonsterHostile(bloat);
}

static void levitation_web_setup(void) {

    setTerrainAroundLoc(player.loc, FLOOR);
    setPlayerPos(player.loc);

    // Place a web under the player's feet.
    setTerrain(player.loc, SPIDERWEB);  // T_ENTANGLES

    // Give the player levitation (it does not protect from webbing).
    applyStatusToMonster(&player, STATUS_LEVITATING, 10);

    // Apply instant tile effects so the web sticks the levitating player.
    applyTileEffectsToPlayer();
}

// Setup, then levitate the player
static void levitation_chasm_setup(void) {
    chasm_setup();
    drink(POTION_LEVITATION);
}

static void levitation_trap_setup(void) {
    trap_setup();
    drink(POTION_LEVITATION);
}

static void levitation_water_setup(void) {
    water_setup();
    drink(POTION_LEVITATION);
}

static void levitation_bog_setup(void) {
    bog_monster_setup();
    drink(POTION_LEVITATION);
}

static void levitation_eel_setup(void) {
    eel_setup();
    drink(POTION_LEVITATION);
}

static void levitation_rat_setup(void) {
    rat_setup();
    drink(POTION_LEVITATION);
}


// ------- STEP FUNCTIONS -------

static boolean levitation_breaks_seized_step(void) {
    const creature *bog = findMonster(MK_BOG_MONSTER);

    // If player is seized, take levitation potion
    if (player.bookkeepingFlags & MB_SEIZED && bog->bookkeepingFlags & MB_SEIZING) {
        // fprintf(stderr, "Player seized, taking levitation potion\n");
        drink(POTION_LEVITATION);
        return true;
    }

    // Let the bog monster attempt to seize player
    if (turnNumber() < 10) {
        // fprintf(stderr, "...resting\n");
        restOneTurn();
        return true;
    }

    return false;
}


// ------- VERIFY FUNCTIONS -------

static boolean levitation_chasm_verify(void) {
    // fprintf(stderr,"depth: %d, stat[lev]: %d, turn: %d, ", rogue.depthLevel, player.status[STATUS_LEVITATING], rogue.absoluteTurnNumber );
    TEST_ASSERT(!isSameDepth() && !monsterHasStatus(&player, STATUS_LEVITATING) && turnNumber() >= TEMP_STATUS_AMOUNT, "Player should have fallen after levitation expired");
    return !testFailed;
}

static boolean chasm_fall_verify(void) {
    // fprintf(stderr,"depth: %d, stat[lev]: %d, turn: %d, ", rogue.depthLevel, player.status[STATUS_LEVITATING], rogue.absoluteTurnNumber );
    TEST_ASSERT(!isSameDepth() && !monsterHasStatus(&player, STATUS_LEVITATING) && turnNumber() < TEMP_STATUS_AMOUNT, "Player should have fallen immediately");
    // TEST_ASSERT(player.bookkeepingFlags & MB_IS_FALLING, "Player should be falling immediately");
    return !testFailed;
}

static boolean trap_not_triggered(void) {
    // Trap should NOT be triggered (pressure plate not depressed)
    TEST_ASSERT(!isTrapTriggered(player.loc), "Pressure plate should not be depressed");
    return !testFailed;
}

static boolean trap_triggered(void) {
    TEST_ASSERT(isTrapTriggered(player.loc), "Pressure plate should be depressed");
    return !testFailed;
}

static boolean player_is_hurt(void) {
    TEST_ASSERT(isPlayerHurt(), "Player should be hurt");
    return !testFailed;
}

static boolean player_is_healthy(void) {
    TEST_ASSERT(isPlayerHealthy(), "Player should be unharmeds");
    return !testFailed;
}

static boolean levitation_web_verify(void) {
    // Player SHOULD become stuck (current game behavior: levitation doesn't prevent web entanglement)
    TEST_ASSERT(player.status[STATUS_STUCK] > 0, "Player should become stuck in web even while levitating (current behavior)");
    return !testFailed;
}

static boolean items_remain_in_inventory(void) {
    return findItem(WEAPON,SPEAR) == findInventoryItem(WEAPON,SPEAR)
        && findItem(WEAPON,MACE)  == findInventoryItem(WEAPON,MACE)
        && findItem(WEAPON,WHIP)  == findInventoryItem(WEAPON,WHIP)
        && findItem(WEAPON,AXE)   == findInventoryItem(WEAPON,AXE)
        && findItem(WEAPON,PIKE)  == findInventoryItem(WEAPON,PIKE);
}

static boolean inventory_items_not_missing(void) {
    TEST_ASSERT(items_remain_in_inventory(), "Items should remain in inventory while levitating over deep water");
    return !testFailed;
}

static boolean inventory_items_missing(void) {
    TEST_ASSERT(!items_remain_in_inventory(), "Items should wash out of inventory while in deep water");
    return !testFailed;
}

static boolean player_seized_verify(void) {
    TEST_ASSERT(player.bookkeepingFlags & MB_SEIZED, "Bog monster should seize player");
    return !testFailed;
}

static boolean player_not_seized_verify(void) {
    TEST_ASSERT(!(player.bookkeepingFlags & MB_SEIZED), "Levitation should break SEIZED status");
    return !testFailed;
}


const testCase tests_examples[] = {
    { "BR16982_chasm_fall",             chasm_setup,                wait_until_fall_step,               chasm_fall_verify           },
    { "BR17312_water_steals_items",     water_setup,                wait_10_turns_step,                 inventory_items_missing     },
    { "BR17384_bog_monster_seizes",     bog_monster_setup,          wait_10_turns_step,                 player_seized_verify        },
    { "BR17765_aquatic_attack",         eel_setup,                  wait_10_turns_step,                 player_is_hurt              },
    { "BR20087_trigger_trap",           trap_setup,                 trap_step,                     trap_triggered              },
    { "BR21016_harmful_gas",            gas_setup,                  wait_10_turns_step,                 player_is_hurt              },

    { "BR22389_levitation_breaks_seized", bog_monster_setup,         levitation_breaks_seized_step, player_not_seized_verify    },
    { "BR23474_levitation_chasm",       levitation_chasm_setup,     wait_until_fall_step,               levitation_chasm_verify,    },
    { "BR25575_regular_can_attack_levitating", levitation_rat_setup, wait_10_turns_step,             player_is_hurt              },
    { "BR37800_aquatic_cant_attack_levitating", levitation_eel_setup, wait_10_turns_step,             player_is_healthy           },
    { "BR39325_levitation_over_water",  levitation_water_setup,     wait_10_turns_step,                 inventory_items_not_missing },
    { "BR39456_levitation_over_trap",   levitation_trap_setup,      trap_step,                     trap_not_triggered          },
    { "BR39871_levitation_web",         levitation_web_setup,       NULL,                      levitation_web_verify       },

    // --- Terminal Test ---
    { NULL, NULL, NULL, NULL, NULL }
};
