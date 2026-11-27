#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

const short TEMP_STATUS_AMOUNT = 5;

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
    pos waterLoc = getPlayerPos();
    setTerrainAroundLoc(waterLoc, DEEP_WATER);
    setPlayerPos(waterLoc); // Replace player on tile

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

static creature *aquatic_eel;

static void eel_setup(void) {
    water_setup();
    clearMonsters();

    // Spawn the eel without forcing it hostile: standing in deep water is what
    // makes it treat the player as an enemy.
    aquatic_eel = spawnMonster(MK_EEL, posOffset(getPlayerPos(), 1, 0));

    // Give the player enough HP to survive the eel's attacks until one lands.
    drink(POTION_LIFE);
}

// A deliberately hostile eel, so a test can isolate a condition (such as
// levitation) that stops an otherwise-willing attacker.
static void hostile_eel_setup(void) {
    eel_setup();
    makeMonsterHostile(aquatic_eel);
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

static void levitation_eel_setup(void) {
    hostile_eel_setup();
    drink(POTION_LEVITATION);
}

static void levitation_rat_setup(void) {
    rat_setup();
    drink(POTION_LEVITATION);
}


// ------- STEP FUNCTIONS -------

static boolean levitation_breaks_seized_step(void) {
    creature *bog = findMonster(MK_BOG_MONSTER);

    if (player.bookkeepingFlags & MB_SEIZED) {
        // If player is seized, take the levitation potion to break the grab.
        drink(POTION_LEVITATION);
        return true;
    }
    if (isPlayerLevitating()) {
        return false; // levitating and free; let verify confirm the grab is gone
    }

    // The grab happens inside the bog monster's attack, so drive one directly;
    // waiting for the monster's erratic movement to land an attack can take an
    // unbounded number of turns.
    TEST_ASSERT(bog != NULL && monsterIsAlive(bog), "bog monster should be present");
    if (testFailed) {
        return false;
    }
    monsterAttacksPlayer(bog);
    return true;
}


// ------- VERIFY FUNCTIONS -------

static boolean levitation_chasm_verify(void) {
    TEST_ASSERT(!isSameDepth() && !monsterHasStatus(&player, STATUS_LEVITATING) && turnNumber() >= TEMP_STATUS_AMOUNT, "Player should have fallen after levitation expired");
    return !testFailed;
}

static boolean chasm_fall_verify(void) {
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

static boolean aquatic_eel_verify(void) {
    TEST_ASSERT(aquatic_eel != NULL && monsterIsAlive(aquatic_eel), "eel spawned");
    TEST_ASSERT(isPlayerHurt(),
                "the eel should attack a creature standing in deep water");
    return !testFailed;
}

static boolean aquatic_eel_cannot_attack_levitating(void) {
    TEST_ASSERT(aquatic_eel != NULL && monsterIsAlive(aquatic_eel), "eel spawned");
    TEST_ASSERT(isPlayerLevitating(), "player should be levitating");
    TEST_ASSERT(aquatic_eel == NULL || !monsterIsAlive(aquatic_eel)
                    || !monsterAttacksPlayer(aquatic_eel),
                "a water-bound monster cannot attack a levitating opponent");
    TEST_ASSERT(isPlayerHealthy(),
                "a water-bound monster should not hurt a levitating opponent");
    return !testFailed;
}

static boolean levitation_web_verify(void) {
    // Player SHOULD become stuck (current game behavior: levitation doesn't prevent web entanglement)
    TEST_ASSERT(player.status[STATUS_STUCK] > 0, "Player should become stuck in web even while levitating (current behavior)");
    return !testFailed;
}

static boolean items_remain_in_inventory(void) {
    return findInventoryItem(WEAPON,SPEAR) != NULL
        && findInventoryItem(WEAPON,MACE)  != NULL
        && findInventoryItem(WEAPON,WHIP)  != NULL
        && findInventoryItem(WEAPON,AXE)   != NULL
        && findInventoryItem(WEAPON,PIKE)  != NULL;
}

static boolean inventory_items_not_missing(void) {
    TEST_ASSERT(items_remain_in_inventory(), "Items should remain in inventory while levitating over deep water");
    return !testFailed;
}

static boolean inventory_items_missing(void) {
    TEST_ASSERT(!items_remain_in_inventory(), "Items should wash out of inventory while in deep water");
    return !testFailed;
}

// A bog monster grabs rather than injuring, and the grab happens inside its
// attack.  Drive the attack directly: waiting for the monster's erratic
// movement (it flits in a random direction a third of the time) to land a
// blow leaves the seize to chance.
static boolean bog_monster_attack_step(void) {
    creature *bog = findMonster(MK_BOG_MONSTER);

    TEST_ASSERT(bog != NULL && monsterIsAlive(bog), "bog monster should be present");
    if (!testFailed) {
        monsterAttacksPlayer(bog);
    }
    return false;
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
    { "BR17312_water_steals_items",     water_setup,                wait_100_turns_step,                 inventory_items_missing     },
    { "BR17384_bog_monster_seizes",     bog_monster_setup,          bog_monster_attack_step,            player_seized_verify        },
    // The eel's blows can miss (the player's armor gives a hit chance below
    // 100%), so wait for one to land instead of assuming a hit within a fixed
    // number of turns.
    { "BR17765_aquatic_attack",         eel_setup,                  wait_until_hurt,                    aquatic_eel_verify          },
    { "BR20087_trigger_trap",           trap_setup,                 trap_step,                     trap_triggered              },
    { "BR21016_harmful_gas",            gas_setup,                  wait_10_turns_step,                 player_is_hurt              },

    { "BR22389_levitation_breaks_seized", bog_monster_setup,         levitation_breaks_seized_step, player_not_seized_verify    },
    { "BR23474_levitation_chasm",       levitation_chasm_setup,     wait_until_fall_step,               levitation_chasm_verify,    },
    { "BR25575_regular_can_attack_levitating", levitation_rat_setup, wait_until_hurt,             player_is_hurt              },
    { "BR37800_aquatic_cant_attack_levitating", levitation_eel_setup, wait_10_turns_step,             aquatic_eel_cannot_attack_levitating },
    { "BR39325_levitation_over_water",  levitation_water_setup,     wait_10_turns_step,                 inventory_items_not_missing },
    { "BR39456_levitation_over_trap",   levitation_trap_setup,      trap_step,                     trap_not_triggered          },
    { "BR39871_levitation_web",         levitation_web_setup,       NULL,                      levitation_web_verify       },

    // --- Terminal Test ---
    { NULL, NULL, NULL, NULL }
};
