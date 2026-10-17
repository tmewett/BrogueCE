#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "Rogue.h"

typedef void (*setupFn)(void);
typedef boolean (*verifyFn)(void);

enum testPhase {
    testSetup,
    testStep,
    testVerify
};

typedef struct {
    const char *name;
    // Called after initializeRogue() and startLevel() - set up dungeon state
    setupFn setup;

    // Called each turn to inject events - return true to continue, false to end test
    boolean (*step)(void);

    // Called after test ends - verify state, return true for PASS
    verifyFn verify;
} testCase;

// Every test source file exports one NULL-terminated array of test cases and
// lists it in the master registry (see Tests.c).  This keeps the tests grouped
// by the rules file they came from.
extern const testCase *const testSuites[];

// Test failure reporting
void testFail(const char *msg);
extern boolean testFailed;

#define TEST_ASSERT(cond, msg) do { if (!(cond)) testFail(msg); } while(0)
#define TEST_ASSERT_EQ(a, b, msg) do { if ((a) != (b)) { char buf[200]; sprintf(buf, "%s (expected %d, got %d)", msg, (int)(b), (int)(a)); testFail(buf); } } while(0)
#define TEST_ASSERT_POS_EQ(a, b, msg) do { if ((a).x != (b).x || (a).y != (b).y) { char buf[200]; sprintf(buf, "%s (expected %d,%d got %d,%d)", msg, (b).x, (b).y, (a).x, (a).y); testFail(buf); } } while(0)
#define TEST_ASSERT_MSG(cond, msg) do { if (!(cond)) { char buf[240]; sprintf(buf, "%s: %s", msg, lastMessage()); testFail(buf); } } while(0)

void runTestByName(const char *name);

// Retry a probabilistic action up to maxAttempts times.  Returns true once the
// action returns true, or false if it never does.
boolean retryAction(int maxAttempts, boolean (*action)(void));

// ---------------------------------------------
// Positions
// ---------------------------------------------

pos posAt(int x, int y);
pos posOffset(pos origin, int dx, int dy);

// ---------------------------------------------
// Input
// ---------------------------------------------

void injectKeystroke(int key, boolean shift, boolean control);
void injectMouseClick(pos loc, boolean rightClick, boolean control, boolean shift);

// Supply a keystroke to the engine as the answer to the next prompt or
// confirmation.  Unlike injectKeystroke, this does not immediately act: the
// engine consumes it when it next asks for input.
void queueKeystroke(int key, boolean shift, boolean control);
void queueConfirmYes(void);
void queueConfirmNo(void);

// Show a two-button confirmation prompt and report whether it was accepted.
boolean askConfirmation(void);

// Turn autopilot on (confirmations are then skipped and treated as "yes").
void enableAutopilot(void);

// Report the dungeon seed, turn number, mode, and version to the player.
void showDungeonSeed(void);
void setDungeonSeedHidden(boolean hidden);
unsigned long long dungeonSeed(void);

// Framework internal: pop the next queued keystroke.  Returns false if none is
// pending.  Used by the engine's input loop while in test mode.
boolean testNextInputEvent(rogueEvent *evt);

// ---------------------------------------------
// Messages
// ---------------------------------------------

// Remember a message shown to the player so tests can assert on it.
void recordTestMessage(const char *msg);
void clearMessages(void);
boolean messageWasShown(const char *substring);
const char *lastMessage(void);

// ---------------------------------------------
// Player actions (events)
// ---------------------------------------------

void restOneTurn(void);
void restTurns(int turns);
void moveDirection(enum directions dir);
void moveRight(void);
void moveLeft(void);
void moveUp(void);
void moveDown(void);
void moveUpLeft(void);
void moveUpRight(void);
void moveDownLeft(void);
void moveDownRight(void);
void searchOneTurn(void);

// Perform a single melee attack against the given monster without ending the
// turn, so the immediate result of the blow can be observed.
boolean playerAttacksMonster(creature *monst);

// ---------------------------------------------
// Terrain (events and predicates)
// ---------------------------------------------

void clearTerrain(pos loc);
void setTerrain(pos loc, short terrainType);
void setTerrainAroundLoc(pos loc, short terrainType);
void spawnFeature(pos loc, short feature);
void revealCell(pos loc);
void placeDownStairs(pos loc);
void placeUpStairs(pos loc);
short terrainAt(pos loc);
boolean cellIsTerrain(pos loc, short terrainType);
boolean cellObstructsPassage(pos loc);
boolean cellIsVisible(pos loc);
boolean cellHasGas(pos loc);
boolean cellIsOnFire(pos loc);
boolean cellIsTrap(pos loc);
boolean isTrapTriggered(pos loc);

// ---------------------------------------------
// Player state (predicates and events)
// ---------------------------------------------

pos getPlayerPos(void);
void setPlayerPos(pos loc);
boolean playerIsAt(pos loc);
boolean playerIsOnTerrain(short terrainType);
boolean playerHasStatus(enum statusEffects status);
void applyStatusToPlayer(enum statusEffects status, int duration);
void removeStatusFromPlayer(enum statusEffects status);
void setPlayerHealth(int hp);
int playerHealth(void);
int playerMaxHealth(void);
void damagePlayer(int amount);
int playerNutrition(void);
void setPlayerNutrition(int nutrition);

boolean isPlayerHurt(void);
boolean isPlayerHealthy(void);
boolean isPlayerDead(void);
boolean isPlayerLevitating(void);
boolean isPlayerBurning(void);
boolean isPlayerConfused(void);
boolean isPlayerParalyzed(void);
boolean isPlayerStuck(void);
boolean isPlayerSubmerged(void);
boolean isPlayerPoisoned(void);
boolean isPlayerWeakened(void);
boolean playerHasFeat(short feat);
boolean playerHasItem(short category, short kind);
void removeAllFood(void);

// Numeric status details, for rules about exact counters and amounts.
int playerStatusTurns(enum statusEffects status);
int playerWeaknessAmount(void);
int playerPoisonConcentration(void);
int playerShield(void);
int playerMovementSpeed(void);
int playerArmorValue(void);

// Apply a status effect through the engine's own routines.
void weakenPlayer(int duration);
void poisonPlayer(int duration, int concentration);
void slowPlayer(int turns);
void hastePlayer(int turns);
void healPlayerPanacea(void);
void applyTileEffectsToPlayer(void);

// Armor donning and removal.
void wearArmor(short kind);
void takeOffArmor(void);

// ---------------------------------------------
// Monsters (predicates and events)
// ---------------------------------------------

creature *spawnMonster(enum monsterTypes type, pos loc);
creature *findMonster(enum monsterTypes type);
void clearMonsters(void);
int monsterCount(void);
void setMonsterPos(creature *monst, pos loc);
void makeMonsterHostile(creature *monst);
void makeMonsterAlly(creature *monst);
void wakeMonster(creature *monst);
void putMonsterToSleep(creature *monst);
void makeMonsterUnaware(creature *monst);
void applyStatusToMonster(creature *monst, enum statusEffects status, int duration);
boolean monsterHasStatus(const creature *monst, enum statusEffects status);
boolean monsterIsHealthy(const creature *monst);
boolean monsterIsHurt(const creature *monst);
boolean monsterIsAlly(const creature *monst);
boolean monsterIsHostile(const creature *monst);
boolean monsterIsAsleep(const creature *monst);
boolean monsterIsWandering(const creature *monst);
boolean monsterIsTrackingPlayer(const creature *monst);
boolean monsterIsFleeing(const creature *monst);
int monsterHealth(const creature *monst);
void setMonsterHealth(creature *monst, int hp);
void damageMonster(creature *monst, int amount);
int monsterTicksUntilTurn(const creature *monst);
void giveMonsterItem(creature *monst, short category, short kind);
pos monsterPosition(const creature *monst);
boolean monsterIsAdjacentToPlayer(const creature *monst);

// Have the given monster make a single melee attack against the player,
// without ending the turn.
boolean monsterAttacksPlayer(creature *monst);

// Give the given monster one turn to act (move, attack, cast, and so on).
void monsterTakesTurn(creature *monst);

// Make one monster a follower of another (used for teammate rules).
void makeMonsterFollow(creature *follower, creature *leader);

int monsterStatusTurns(const creature *monst, enum statusEffects status);
int monsterWeaknessAmount(const creature *monst);
int monsterMovementSpeed(const creature *monst);
void weakenMonster(creature *monst, int duration);

// Bestiary trait/ability predicates (read from monsterCatalog info).
boolean monsterHasTrait(const creature *monst, unsigned long trait);
boolean monsterHasAbility(const creature *monst, unsigned long ability);
boolean monsterCastsBolt(const creature *monst, short boltType);

// Base stat predicates (from monsterCatalog info).
int monsterMaxHealth(const creature *monst);
int monsterBaseDefense(const creature *monst);
int monsterBaseAccuracy(const creature *monst);
int monsterBaseDamageLow(const creature *monst);
int monsterBaseDamageHigh(const creature *monst);
int monsterBaseMovementSpeed(const creature *monst);
int monsterBaseAttackSpeed(const creature *monst);
void poisonMonster(creature *monst, int duration, int concentration);
void slowMonster(creature *monst, int turns);
void hasteMonster(creature *monst, int turns);

void setMonsterSubmerged(creature *monst, boolean submerged);
void makeMonsterCaptive(creature *monst);
void makeMonsterDormant(creature *monst);

boolean monsterIsHiddenFrom(const creature *monst, const creature *observer);
boolean monsterIsRevealedToPlayer(const creature *monst);
boolean monsterCouldAttack(const creature *attacker, const creature *defender);
boolean monstersAreAlliesOf(const creature *a, const creature *b);
boolean monstersAreFoesOf(const creature *a, const creature *b);
boolean monsterAvoidsCell(const creature *monst, pos loc);

// ---------------------------------------------
// Items (predicates and events)
// ---------------------------------------------

item *findInventoryItem(short category, short kind);
item *findFloorItem(short category, short kind);
item *findItem(short category, short kind);
item *givePlayerItem(short category, short kind, short enchant);
item *placeItemOnFloor(short category, short kind, pos loc);
void equipItemFromPack(short category, short kind);
// Drink a potion, applying its effect without ending the turn.
void drink(enum potionKind kind);
// Read a scroll, applying its effect (and ending the turn).
void readScrollOfKind(short kind);

// Item details a rule can depend on.
int itemQuantity(const item *theItem);
int itemCharges(const item *theItem);
int itemEnchant(const item *theItem);
char itemInventoryLetter(const item *theItem);
short itemKind(const item *theItem);
short itemCategory(const item *theItem);
boolean itemIsCursed(const item *theItem);
boolean itemIsRunic(const item *theItem);
unsigned long itemGoldValue(const item *theItem);
boolean itemKindIsKnown(short category, short kind);

// The items currently worn.
item *equippedWeapon(void);
item *equippedArmor(void);

// Counts within the pack.
int packStackCount(short category, short kind);
int packUnitCount(short category, short kind);
int packItemCount(void);

// Player numbers related to items.
int playerGold(void);
int playerStrength(void);
int playerDefense(void);

// Damage the player through the engine's own damage routine, so rules about
// damage reduction (such as easy mode) apply.
void woundPlayer(int amount);

// Succumb to easy mode.
void enterEasyMode(void);

// Wear a freshly generated suit of armor at a chosen enchantment.
void wearArmorOfEnchant(short kind, int enchant);

// Create a freshly generated runic weapon/armor in the player's pack at the
// chosen enchantment, already identified as runic.
item *givePlayerRunicWeapon(short kind, short runicKind, short enchant);
item *givePlayerRunicArmor(short kind, short runicKind, short enchant);

// ---------------------------------------------
// Dungeon (predicates and events)
// ---------------------------------------------

// The fixed dimensions of a dungeon level, in tiles.
int levelWidth(void);
int levelHeight(void);

// How many tiles of the dungeon layer hold the given terrain.
int countTerrain(short terrainType);

// Descend one level, generating the next dungeon level.
void descendToNextLevel(void);

// The total brightness of a tile's light.
int lightAt(pos loc);

// Pathfinding distances between two tiles.  pathingDistanceBetween routes with
// no traveller (so undiscovered tiles are passable); playerPathingDistance
// routes as the player (so undiscovered tiles are impassable).
int pathingDistanceBetween(pos from, pos to, unsigned long blockingTerrainFlags);
int playerPathingDistance(pos from, pos to);

// Timed lighting.
int minersLightRadius(void);
void setLightMultiplier(int multiplier);
void setPlayerInWater(boolean inWater);
void recomputeLighting(void);

// ---------------------------------------------
// World (predicates and events)
// ---------------------------------------------

unsigned long turnNumber(void);
int ticksUntilPlayerActs(void);
int currentDepth(void);
void setDepth(int depth);
boolean isSameDepth(void);
boolean playerIsCarryingAmulet(void);
int deepestDepth(void);
int totalGoldGenerated(void);
int playerTurnCount(void);

#endif
