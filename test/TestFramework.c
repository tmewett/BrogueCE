#include "TestFramework.h"
#include "Rogue.h"
#include "Globals.h"
#include "GlobalsBase.h"
#include <string.h>
#include <stdio.h>

// The master registry of test suites (see Tests.c).
extern const testCase *const testSuites[];

static int testStepCount = 0;
static short testStartDepth = 1;
boolean testFailed = false;
static char failureMessage[COLS * 3];

// Queue of keystrokes that the engine will consume as input.  In test mode the
// engine never reads the real console, so answers to confirmations and other
// prompts are supplied here.  When the queue is empty the engine behaves as if
// the player pressed escape, so a test can never hang on a prompt.
#define TEST_INPUT_QUEUE_SIZE 256
static rogueEvent inputQueue[TEST_INPUT_QUEUE_SIZE];
static int inputQueueHead = 0;
static int inputQueueTail = 0;

// Messages shown to the player during the current test.
#define TEST_MESSAGE_LOG_SIZE 64
static char messageLog[TEST_MESSAGE_LOG_SIZE][DCOLS * 2];
static int messageLogCount = 0;

void queueKeystroke(int key, boolean shift, boolean control) {
    int next = (inputQueueTail + 1) % TEST_INPUT_QUEUE_SIZE;
    if (next == inputQueueHead) {
        return; // Queue full; drop the keystroke.
    }
    rogueEvent *evt = &inputQueue[inputQueueTail];
    memset(evt, 0, sizeof(*evt));
    evt->eventType = KEYSTROKE;
    evt->param1 = key;
    evt->shiftKey = shift;
    evt->controlKey = control;
    inputQueueTail = next;
}

boolean testNextInputEvent(rogueEvent *evt) {
    if (inputQueueHead == inputQueueTail) {
        return false;
    }
    *evt = inputQueue[inputQueueHead];
    inputQueueHead = (inputQueueHead + 1) % TEST_INPUT_QUEUE_SIZE;
    return true;
}

void queueConfirmYes(void) {
    queueKeystroke('y', false, false);
}

void queueConfirmNo(void) {
    queueKeystroke('n', false, false);
}

boolean askConfirmation(void) {
    return confirm("Are you sure?", false);
}

void enableAutopilot(void) {
    rogue.autoPlayingLevel = true;
}

void showDungeonSeed(void) {
    printSeed();
}

void setDungeonSeedHidden(boolean hidden) {
    rogue.hideSeed = hidden;
}

unsigned long long dungeonSeed(void) {
    return rogue.seed;
}

static void clearInputQueue(void) {
    inputQueueHead = inputQueueTail = 0;
}

void recordTestMessage(const char *msg) {
    if (messageLogCount < TEST_MESSAGE_LOG_SIZE) {
        messageLogCount++;
    }
    // Shift older messages down so the newest is always last.
    int start = (messageLogCount == TEST_MESSAGE_LOG_SIZE) ? 1 : 0;
    memmove(messageLog[0], messageLog[start],
            sizeof(messageLog[0]) * (messageLogCount - 1 - start));
    snprintf(messageLog[messageLogCount - 1], sizeof(messageLog[0]), "%s", msg);
}

void clearMessages(void) {
    messageLogCount = 0;
}

boolean messageWasShown(const char *substring) {
    for (int i = 0; i < messageLogCount; i++) {
        if (strstr(messageLog[i], substring) != NULL) {
            return true;
        }
    }
    return false;
}

const char *lastMessage(void) {
    if (messageLogCount == 0) {
        return "(no message)";
    }
    return messageLog[messageLogCount - 1];
}

void runTestByName(const char *name) {
    testStartDepth = rogue.depthLevel;

    for (const testCase *const *suite = testSuites; *suite != NULL; suite++) {
        for (const testCase *currentTest = *suite; currentTest->name != NULL; currentTest++) {
            if (strcmp(currentTest->name, name) != 0) {
                continue;
            }

            testStepCount = 0;
            testFailed = false;
            failureMessage[0] = '\0';
            clearInputQueue();
            clearMessages();

            // Setup
            if (currentTest->setup) currentTest->setup();

            // Run steps until test ends or game ends
            while (!rogue.gameHasEnded && !testFailed) {
                if (!currentTest->step || !currentTest->step()) {
                    break; // Test signaled completion
                }
                testStepCount++;
                if (testStepCount > 10000) { // Safety limit
                    testFail("Test exceeded 10000 steps");
                    break;
                }
            }

            // Verify
            boolean passed = true;
            if (currentTest->verify) {
                passed = currentTest->verify();
            }

            printf("\n=== TEST %s: %s ===\n", name, passed && !testFailed ? "PASS" : "FAIL");
            if (testFailed) {
                printf("        Failure: %s\n", failureMessage);
            }

            rogue.gameHasEnded = true; // Exit game after test
            rogue.nextGame = NG_QUIT;
            return;
        }
    }
    printf("Test not found: %s\n", name);
    rogue.nextGame = NG_QUIT;
}

void testFail(const char *msg) {
    testFailed = true;
    snprintf(failureMessage, sizeof(failureMessage), "%s", msg);
}

// ---------------------------------------------
// Positions
// ---------------------------------------------

pos posAt(int x, int y) {
    pos loc;
    loc.x = x;
    loc.y = y;
    return loc;
}

pos posOffset(pos origin, int dx, int dy) {
    return posAt(origin.x + dx, origin.y + dy);
}

// ---------------------------------------------
// Event Injection
// ---------------------------------------------

void injectKeystroke(int key, boolean shift, boolean control) {
    rogueEvent evt = {0};
    evt.eventType = KEYSTROKE;
    evt.param1 = key;
    evt.shiftKey = shift;
    evt.controlKey = control;
    executeEvent(&evt);
}

void injectMouseClick(pos loc, boolean rightClick, boolean control, boolean shift) {
    rogueEvent evt = {0};
    evt.eventType = rightClick ? RIGHT_MOUSE_UP : MOUSE_UP;
    evt.param1 = loc.x;
    evt.param2 = loc.y;
    evt.controlKey = control;
    evt.shiftKey = shift;
    executeMouseClick(&evt);
}

void restOneTurn(void) {
    injectKeystroke(PERIOD_KEY, false, false);
}

void restTurns(int turns) {
    for (int i = 0; i < turns && !rogue.gameHasEnded; i++) {
        restOneTurn();
    }
}

void moveDirection(enum directions dir) {
    static const int keys[] = {
        [UP] = UP_KEY, [DOWN] = DOWN_KEY, [LEFT] = LEFT_KEY, [RIGHT] = RIGHT_KEY,
        [UPLEFT] = UPLEFT_KEY, [UPRIGHT] = UPRIGHT_KEY,
        [DOWNLEFT] = DOWNLEFT_KEY, [DOWNRIGHT] = DOWNRIGHT_KEY
    };
    if (dir >= UP && dir <= DOWNRIGHT) {
        injectKeystroke(keys[dir], false, false);
    }
}

void moveRight(void)      { moveDirection(RIGHT); }
void moveLeft(void)       { moveDirection(LEFT); }
void moveUp(void)         { moveDirection(UP); }
void moveDown(void)       { moveDirection(DOWN); }
void moveUpLeft(void)     { moveDirection(UPLEFT); }
void moveUpRight(void)    { moveDirection(UPRIGHT); }
void moveDownLeft(void)   { moveDirection(DOWNLEFT); }
void moveDownRight(void)  { moveDirection(DOWNRIGHT); }

void searchOneTurn(void) {
    injectKeystroke(SEARCH_KEY, false, false);
}

boolean playerAttacksMonster(creature *monst) {
    if (monst == NULL) {
        return false;
    }
    return attack(&player, monst, false);
}

// ---------------------------------------------
// Terrain
// ---------------------------------------------

void clearTerrain(pos loc) {
    pcell *cell = &(pmap[loc.x][loc.y]);
    memset(cell, 0, sizeof(pcell));   // Clears everything (layers, gas, items, etc.)
}

void setTerrain(pos loc, short terrainType) {
    clearTerrain(loc);
    pmap[loc.x][loc.y].layers[DUNGEON] = terrainType;
    refreshDungeonCell(loc);
}

void setTerrainAroundLoc(pos loc, short terrainType) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            setTerrain(posOffset(loc, dx, dy), terrainType);
        }
    }
}

void spawnFeature(pos loc, short feature) {
    spawnDungeonFeature(loc.x, loc.y, &dungeonFeatureCatalog[feature], true, false);
}

void revealCell(pos loc) {
    // Mark a cell as seen so tests can exercise the "known terrain" prompts.
    pmapAt(loc)->flags |= (VISIBLE | DISCOVERED);
    refreshDungeonCell(loc);
}

void placeDownStairs(pos loc) {
    setTerrain(loc, DOWN_STAIRS);
    rogue.downLoc = loc;
}

void placeUpStairs(pos loc) {
    setTerrain(loc, UP_STAIRS);
    rogue.upLoc = loc;
}

short terrainAt(pos loc) {
    return pmapAt(loc)->layers[DUNGEON];
}

boolean cellIsTerrain(pos loc, short terrainType) {
    for (int layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (pmapAt(loc)->layers[layer] == terrainType) {
            return true;
        }
    }
    return false;
}

boolean cellObstructsPassage(pos loc) {
    return cellHasTerrainFlag(loc, T_OBSTRUCTS_PASSABILITY);
}

boolean cellIsVisible(pos loc) {
    return playerCanSee(loc.x, loc.y);
}

boolean cellHasGas(pos loc) {
    return pmapAt(loc)->volume > 0;
}

boolean cellIsOnFire(pos loc) {
    return cellHasTerrainFlag(loc, T_IS_FIRE);
}

boolean cellIsTrap(pos loc) {
    return cellHasTerrainFlag(loc, T_IS_DF_TRAP);
}

boolean isTrapTriggered(pos loc) {
    return (pmapAt(loc)->flags & PRESSURE_PLATE_DEPRESSED) != 0;
}

// ---------------------------------------------
// Player state
// ---------------------------------------------

pos getPlayerPos(void) {
    return player.loc;
}

void setPlayerPos(pos loc) {
    setMonsterPos(&player, loc);
}

boolean playerIsAt(pos loc) {
    return player.loc.x == loc.x && player.loc.y == loc.y;
}

boolean playerIsOnTerrain(short terrainType) {
    return cellIsTerrain(player.loc, terrainType);
}

boolean playerHasStatus(enum statusEffects status) {
    return player.status[status] > 0;
}

void applyStatusToPlayer(enum statusEffects status, int duration) {
    player.status[status] = player.maxStatus[status] = duration;
}

void removeStatusFromPlayer(enum statusEffects status) {
    player.status[status] = player.maxStatus[status] = 0;
}

void setPlayerHealth(int hp) {
    if (hp < 1) hp = 1;
    if (hp > player.info.maxHP) hp = player.info.maxHP;
    player.currentHP = hp;
}

int playerHealth(void) {
    return player.currentHP;
}

int playerMaxHealth(void) {
    return player.info.maxHP;
}

void damagePlayer(int amount) {
    player.currentHP -= amount;
    if (player.currentHP < 0) {
        player.currentHP = 0;
    }
}

void woundPlayer(int amount) {
    inflictDamage(NULL, &player, amount, NULL, true);
}

void enterEasyMode(void) {
    rogue.mode = GAME_MODE_EASY;
}

int playerNutrition(void) {
    return player.status[STATUS_NUTRITION];
}

void setPlayerNutrition(int nutrition) {
    player.status[STATUS_NUTRITION] = nutrition;
}

boolean isPlayerHurt(void) {
    return player.currentHP < player.info.maxHP;
}

boolean isPlayerHealthy(void) {
    return player.currentHP >= player.info.maxHP;
}

boolean isPlayerDead(void) {
    return player.currentHP <= 0;
}

boolean isPlayerLevitating(void) {
    return playerHasStatus(STATUS_LEVITATING);
}

boolean isPlayerBurning(void) {
    return playerHasStatus(STATUS_BURNING);
}

boolean isPlayerConfused(void) {
    return playerHasStatus(STATUS_CONFUSED);
}

boolean isPlayerParalyzed(void) {
    return playerHasStatus(STATUS_PARALYZED);
}

boolean isPlayerStuck(void) {
    return playerHasStatus(STATUS_STUCK);
}

boolean isPlayerSubmerged(void) {
    // The player's submersion is tracked as a visual state, unlike monsters.
    return rogue.inWater != 0;
}

boolean isPlayerPoisoned(void) {
    return player.status[STATUS_POISONED] > 0;
}

boolean isPlayerWeakened(void) {
    return player.status[STATUS_WEAKENED] > 0;
}

boolean playerHasFeat(short feat) {
    return rogue.featRecord[feat] != 0;
}

int playerStatusTurns(enum statusEffects status) {
    return player.status[status];
}

int playerWeaknessAmount(void) {
    return player.weaknessAmount;
}

int playerPoisonConcentration(void) {
    return player.poisonAmount;
}

int playerShield(void) {
    return player.status[STATUS_SHIELDED];
}

int playerMovementSpeed(void) {
    return player.movementSpeed;
}

int playerArmorValue(void) {
    return rogue.armor ? rogue.armor->armor : 0;
}

void weakenPlayer(int duration) {
    weaken(&player, duration);
}

void poisonPlayer(int duration, int concentration) {
    addPoison(&player, duration, concentration);
}

void slowPlayer(int turns) {
    slow(&player, turns);
}

void hastePlayer(int turns) {
    haste(&player, turns);
}

void healPlayerPanacea(void) {
    heal(&player, 100, true);
}

void applyTileEffectsToPlayer(void) {
    applyInstantTileEffectsToCreature(&player);
}

void wearArmor(short kind) {
    item *theItem = givePlayerItem(ARMOR, kind, 0);
    if (theItem != NULL) {
        equipItem(theItem, false, NULL);
    }
}

void takeOffArmor(void) {
    if (rogue.armor != NULL) {
        unequipItem(rogue.armor, true);
    }
}

// ---------------------------------------------
// Monsters
// ---------------------------------------------

void clearMonsters(void) {
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *m = nextCreature(&it);
        killCreature(m, true);
    }
}

creature *spawnMonster(enum monsterTypes type, pos loc) {
    creature *monst = generateMonster(type, false, false);
    setMonsterPos(monst, loc);
    return monst;
}

creature *findMonster(enum monsterTypes type) {
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *m = nextCreature(&it);
        if (m->info.monsterID == type) return m;
    }
    return NULL;
}

int monsterCount(void) {
    int count = 0;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        nextCreature(&it);
        count++;
    }
    return count;
}

void setMonsterPos(creature *monst, pos loc) {
    unsigned long creatureFlag = (monst == &player ? HAS_PLAYER : HAS_MONSTER);
    pmapAt(monst->loc)->flags &= ~creatureFlag;
    refreshDungeonCell(monst->loc);

    monst->loc = loc;
    pmapAt(monst->loc)->flags |= creatureFlag;
    refreshDungeonCell(monst->loc);
}

void makeMonsterHostile(creature *monst) {
    monst->creatureState = MONSTER_TRACKING_SCENT;
}

void makeMonsterAlly(creature *monst) {
    monst->creatureState = MONSTER_ALLY;
}

void wakeMonster(creature *monst) {
    monst->creatureState = MONSTER_TRACKING_SCENT;
}

void putMonsterToSleep(creature *monst) {
    monst->creatureState = MONSTER_SLEEPING;
}

void makeMonsterUnaware(creature *monst) {
    monst->creatureState = MONSTER_WANDERING;
}

void applyStatusToMonster(creature *monst, enum statusEffects status, int duration) {
    monst->status[status] = monst->maxStatus[status] = duration;
    switch (status) {
        case STATUS_POISONED: monst->poisonAmount = 1; break;
        case STATUS_WEAKENED: monst->weaknessAmount = 1; break;
        default: break;
    }
}

boolean monsterHasStatus(const creature *monst, enum statusEffects status) {
    return monst->status[status] > 0;
}

boolean monsterIsHealthy(const creature *monst) {
    int max = monst->info.maxHP * 0.95;
    return monst->currentHP >= max;
}

boolean monsterIsHurt(const creature *monst) {
    return !monsterIsHealthy(monst);
}

boolean monsterIsAlly(const creature *monst) {
    return monst->creatureState == MONSTER_ALLY;
}

boolean monsterIsHostile(const creature *monst) {
    return monst != &player && monst->creatureState != MONSTER_ALLY;
}

boolean monsterIsAsleep(const creature *monst) {
    return monst->creatureState == MONSTER_SLEEPING;
}

boolean monsterIsWandering(const creature *monst) {
    return monst->creatureState == MONSTER_WANDERING;
}

boolean monsterIsTrackingPlayer(const creature *monst) {
    return monst->creatureState == MONSTER_TRACKING_SCENT;
}

boolean monsterIsFleeing(const creature *monst) {
    return monst->creatureState == MONSTER_FLEEING;
}

int monsterTicksUntilTurn(const creature *monst) {
    return monst->ticksUntilTurn;
}

void giveMonsterItem(creature *monst, short category, short kind) {
    monst->carriedItem = generateItem(category, kind);
}

pos monsterPosition(const creature *monst) {
    return monst->loc;
}

int monsterHealth(const creature *monst) {
    return monst->currentHP;
}

void setMonsterHealth(creature *monst, int hp) {
    if (hp < 1) hp = 1;
    if (hp > monst->info.maxHP) hp = monst->info.maxHP;
    monst->currentHP = hp;
}

void damageMonster(creature *monst, int amount) {
    inflictDamage(NULL, monst, amount, NULL, true);
}

boolean monsterIsAdjacentToPlayer(const creature *monst) {
    int dx = monst->loc.x - player.loc.x;
    int dy = monst->loc.y - player.loc.y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    int chebyshev = (dx > dy ? dx : dy);
    return chebyshev == 1;
}

boolean monsterAttacksPlayer(creature *monst) {
    if (monst == NULL) {
        return false;
    }
    return attack(monst, &player, false);
}

void monsterTakesTurn(creature *monst) {
    if (monst != NULL) {
        monstersTurn(monst);
    }
}

void makeMonsterFollow(creature *follower, creature *leader) {
    if (follower != NULL && leader != NULL) {
        follower->bookkeepingFlags |= MB_FOLLOWER;
        follower->leader = leader;
    }
}

int monsterStatusTurns(const creature *monst, enum statusEffects status) {
    return monst->status[status];
}

int monsterWeaknessAmount(const creature *monst) {
    return monst->weaknessAmount;
}

int monsterMovementSpeed(const creature *monst) {
    return monst->movementSpeed;
}

boolean monsterHasTrait(const creature *monst, unsigned long trait) {
    if (monst == NULL) return false;
    return (monst->info.flags & trait) != 0;
}

boolean monsterHasAbility(const creature *monst, unsigned long ability) {
    if (monst == NULL) return false;
    return (monst->info.abilityFlags & ability) != 0;
}

boolean monsterCastsBolt(const creature *monst, short boltType) {
    if (monst == NULL) return false;
    for (int i = 0; i < 20; i++) {
        if (monst->info.bolts[i] == boltType) return true;
        if (monst->info.bolts[i] == 0) break;
    }
    return false;
}

int monsterMaxHealth(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.maxHP;
}

int monsterBaseDefense(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.defense;
}

int monsterBaseAccuracy(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.accuracy;
}

int monsterBaseDamageLow(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.damage.lowerBound;
}

int monsterBaseDamageHigh(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.damage.upperBound;
}

int monsterBaseMovementSpeed(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.movementSpeed;
}

int monsterBaseAttackSpeed(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.attackSpeed;
}

void weakenMonster(creature *monst, int duration) {
    if (monst != NULL) {
        weaken(monst, duration);
    }
}

void poisonMonster(creature *monst, int duration, int concentration) {
    if (monst != NULL) {
        addPoison(monst, duration, concentration);
    }
}

void slowMonster(creature *monst, int turns) {
    if (monst != NULL) {
        slow(monst, turns);
    }
}

void hasteMonster(creature *monst, int turns) {
    if (monst != NULL) {
        haste(monst, turns);
    }
}

void setMonsterSubmerged(creature *monst, boolean submerged) {
    if (monst == NULL) {
        return;
    }
    if (submerged) {
        monst->bookkeepingFlags |= MB_SUBMERGED;
    } else {
        monst->bookkeepingFlags &= ~MB_SUBMERGED;
    }
}

void makeMonsterCaptive(creature *monst) {
    if (monst != NULL) {
        monst->bookkeepingFlags |= MB_CAPTIVE;
    }
}

void makeMonsterDormant(creature *monst) {
    if (monst != NULL) {
        monst->bookkeepingFlags |= MB_IS_DORMANT;
    }
}

boolean monsterIsHiddenFrom(const creature *monst, const creature *observer) {
    return monsterIsHidden(monst, observer);
}

boolean monsterIsRevealedToPlayer(const creature *monst) {
    return monsterRevealed((creature *)monst);
}

boolean monsterCouldAttack(const creature *attacker, const creature *defender) {
    return monsterWillAttackTarget(attacker, defender);
}

boolean monstersAreAlliesOf(const creature *a, const creature *b) {
    return monstersAreTeammates(a, b);
}

boolean monstersAreFoesOf(const creature *a, const creature *b) {
    return monstersAreEnemies(a, b);
}

boolean monsterAvoidsCell(const creature *monst, pos loc) {
    return monsterAvoids((creature *)monst, loc);
}

// ---------------------------------------------
// Items
// ---------------------------------------------

item *findInventoryItem(short category, short kind) {
    for (item *i = packItems->nextItem; i; i = i->nextItem) {
        if (i->category == category && i->kind == kind) return i;
    }
    return NULL;
}

item *findFloorItem(short category, short kind) {
    for (item *i = floorItems->nextItem; i; i = i->nextItem) {
        if (i->category == category && i->kind == kind) return i;
    }
    return NULL;
}

item *findItem(short category, short kind) {
    item *i = findInventoryItem(category, kind);
    if (i != NULL) return i;
    return findFloorItem(category, kind);
}

item *givePlayerItem(short category, short kind, short enchant) {
    item *theItem = generateItem(category, kind);
    if (enchant) theItem->enchant1 = enchant;
    identify(theItem);
    return addItemToPack(theItem);
}

item *placeItemOnFloor(short category, short kind, pos loc) {
    item *theItem = generateItem(category, kind);
    return placeItemAt(theItem, loc);
}

void equipItemFromPack(short category, short kind) {
    item *theItem = findInventoryItem(category, kind);
    if (theItem != NULL) {
        equipItem(theItem, true, NULL);
    }
}

void drink(enum potionKind kind) {
    // Apply the potion's effect without ending the turn, so that transient
    // results (such as a cloud of gas) can be observed.  A paralyzing cloud
    // would otherwise fully disperse during the turns that automatically pass
    // while you are paralyzed.
    item *potion = givePlayerItem(POTION, kind, 0);
    if (potion != NULL) {
        drinkPotion(potion);
    }
}

void readScrollOfKind(short kind) {
    item *scroll = givePlayerItem(SCROLL, kind, 0);
    apply(scroll);
}

int itemQuantity(const item *theItem) {
    return theItem->quantity;
}

int itemCharges(const item *theItem) {
    return theItem->charges;
}

int itemEnchant(const item *theItem) {
    return theItem->enchant1;
}

char itemInventoryLetter(const item *theItem) {
    return theItem->inventoryLetter;
}

short itemKind(const item *theItem) {
    return theItem->kind;
}

short itemCategory(const item *theItem) {
    return theItem->category;
}

boolean itemIsCursed(const item *theItem) {
    return (theItem->flags & ITEM_CURSED) != 0;
}

boolean itemIsRunic(const item *theItem) {
    return (theItem->flags & ITEM_RUNIC) != 0;
}

item *equippedWeapon(void) {
    return rogue.weapon;
}

item *equippedArmor(void) {
    return rogue.armor;
}

unsigned long itemGoldValue(const item *theItem) {
    return itemValue((item *)theItem);
}

boolean itemKindIsKnown(short category, short kind) {
    return tableForItemCategory(category)[kind].identified != 0;
}

static int matchingPackItems(short category, short kind, boolean sumUnits) {
    int total = 0;
    for (item *i = packItems->nextItem; i != NULL; i = i->nextItem) {
        if (i->category == category && i->kind == kind) {
            total += sumUnits ? i->quantity : 1;
        }
    }
    return total;
}

int packStackCount(short category, short kind) {
    return matchingPackItems(category, kind, false);
}

int packUnitCount(short category, short kind) {
    return matchingPackItems(category, kind, true);
}

int packItemCount(void) {
    return numberOfItemsInPack();
}

int playerGold(void) {
    return rogue.gold;
}

int playerStrength(void) {
    return rogue.strength;
}

int playerDefense(void) {
    return player.info.defense;
}

void wearArmorOfEnchant(short kind, int enchant) {
    item *theItem = generateItem(ARMOR, kind);
    theItem->enchant1 = enchant;
    theItem->enchant2 = 0;
    theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    identify(theItem);
    addItemToPack(theItem);
    equipItem(theItem, true, NULL);
}

item *givePlayerRunicWeapon(short kind, short runicKind, short enchant) {
    item *theItem = generateItem(WEAPON, kind);
    theItem->enchant1 = enchant;
    theItem->enchant2 = runicKind;
    theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    theItem->flags &= ~ITEM_CURSED;
    identify(theItem);
    addItemToPack(theItem);
    return theItem;
}

item *givePlayerRunicArmor(short kind, short runicKind, short enchant) {
    item *theItem = generateItem(ARMOR, kind);
    theItem->enchant1 = enchant;
    theItem->enchant2 = runicKind;
    theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    theItem->flags &= ~ITEM_CURSED;
    identify(theItem);
    addItemToPack(theItem);
    equipItem(theItem, true, NULL);
    return theItem;
}

boolean playerHasItem(short category, short kind) {
    return findInventoryItem(category, kind) != NULL;
}

void removeAllFood(void) {
    // Starving players automatically eat any food in the pack, so a test of
    // starvation has to get rid of it first.
    for (item *i = packItems->nextItem; i != NULL;) {
        item *next = i->nextItem;
        if (i->category == FOOD) {
            removeItemFromChain(i, packItems);
            deleteItem(i);
        }
        i = next;
    }
}

// ---------------------------------------------
// World
// ---------------------------------------------

unsigned long turnNumber(void) {
    return rogue.absoluteTurnNumber;
}

int ticksUntilPlayerActs(void) {
    return player.ticksUntilTurn;
}

int currentDepth(void) {
    return rogue.depthLevel;
}

void setDepth(int depth) {
    rogue.depthLevel = depth;
}

boolean isSameDepth(void) {
    return rogue.depthLevel == testStartDepth;
}

boolean playerIsCarryingAmulet(void) {
    return findInventoryItem(AMULET, 0) != NULL;
}

int deepestDepth(void) {
    return rogue.deepestLevel;
}

int totalGoldGenerated(void) {
    return rogue.goldGenerated;
}

int playerTurnCount(void) {
    return rogue.playerTurnNumber;
}

// ---------------------------------------------
// Dungeon
// ---------------------------------------------

int levelWidth(void) {
    return DCOLS;
}

int levelHeight(void) {
    return DROWS;
}

int countTerrain(short terrainType) {
    int count = 0;
    for (int i = 0; i < DCOLS; i++) {
        for (int j = 0; j < DROWS; j++) {
            if (pmap[i][j].layers[DUNGEON] == terrainType) {
                count++;
            }
        }
    }
    return count;
}

int lightAt(pos loc) {
    return tmapAt(loc)->light[0] + tmapAt(loc)->light[1] + tmapAt(loc)->light[2];
}

void descendToNextLevel(void) {
    rogue.depthLevel++;
    startLevel(rogue.depthLevel - 1, 1);
    if (rogue.depthLevel > rogue.deepestLevel) {
        rogue.deepestLevel = rogue.depthLevel;
    }
}

int pathingDistanceBetween(pos from, pos to, unsigned long blockingTerrainFlags) {
    return pathingDistance(from.x, from.y, to.x, to.y, blockingTerrainFlags);
}

int playerPathingDistance(pos from, pos to) {
    short **distanceMap = allocGrid();
    calculateDistances(distanceMap, to.x, to.y, 0, &player, true, true);
    int distance = distanceMap[from.x][from.y];
    freeGrid(distanceMap);
    return distance;
}

int minersLightRadius(void) {
    updateMinersLightRadius();
    return rogue.minersLight.lightRadius.lowerBound;
}

void setLightMultiplier(int multiplier) {
    rogue.lightMultiplier = multiplier;
}

void setPlayerInWater(boolean inWater) {
    rogue.inWater = inWater;
}

void recomputeLighting(void) {
    updateMinersLightRadius();
    updateLighting();
}

boolean retryAction(int maxAttempts, boolean (*action)(void)) {
    for (int i = 0; i < maxAttempts; i++) {
        if (action()) return true;
    }
    return false;
}
