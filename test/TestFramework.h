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

// Whether the keyboard cursor is currently turned on (Return/Enter toggles it
// on; movement, space or escape turns it off again).
boolean isKeyboardCursorActive(void);

// Supply a keystroke to the engine as the answer to the next prompt or
// confirmation.  Unlike injectKeystroke, this does not immediately act: the
// engine consumes it when it next asks for input.
void queueKeystroke(int key, boolean shift, boolean control);
// Supply a mouse click to the engine as the answer to the next prompt or
// menu.  Like queueKeystroke, the event is consumed when the engine next
// asks for input, so a prompt that is not open yet can still be clicked.
void queueMouseClick(pos loc);
void queueConfirmYes(void);
void queueConfirmNo(void);

// Show a two-button confirmation prompt and report whether it was accepted.
boolean askConfirmation(void);

// Open the engine's file-picker dialog (the one the main menu uses for saved
// games and recordings) with the given filename suffix, and report whether it
// selected a file.  The dialog's answers come from the test's queued input.
boolean chooseFileWithSuffix(const char *suffix);

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
void printTestMessages(void);

// Whether any message the engine emitted while the game was starting up (the
// welcome message and its fellows, all shown before the test began) contains
// the given substring.  runTestByName() clears the ordinary message log before
// each test, so startup messages are kept in their own snapshot.
boolean startupMessageWasShown(const char *substring);
// The full text of the first startup message containing the given substring,
// or NULL when there is none.
const char *startupMessageWith(const char *substring);

// Send one message through the game's real message archive pipeline (repeat
// collapsing, counting, and display formatting).  In test mode the engine
// captures raw message text and skips all of that, so this briefly runs the
// normal-mode path for the single message.
void emitArchivedMessage(const char *msg);

// The newest message line as the game renders it after formatting: repeats
// collapsed into a counted entry ("... (x5)"), "(many)" once the repeat cap
// is reached, capitalized and punctuated.  Empty string if nothing has been
// emitted with emitArchivedMessage() yet.
const char *renderedMessageLine(void);

// The colour the message area shows for a recorded message containing
// `substring`: "bad" (harm to you or an ally), "good" (harm to an enemy), or
// "white" (a neutral event, or a message that carries no colour of its own).
// NULL when no recorded message contains the substring.
const char *messageColorOf(const char *substring);

// ---------------------------------------------
// Screen
// ---------------------------------------------

// Redraw the sidebar and report whether the given text appears in it.  The
// sidebar is the stats column at the left edge of the screen, so a match here
// can only have come from the sidebar.
boolean sidebarContains(const char *substring);

// Report whether the given text appears on the whole screen as of the most
// recent time the engine asked the test for input.  Full-screen displays (the
// discoveries screen, the message archive, the help screen) draw themselves
// over the map, wait for a key, and then restore the previous screen, so the
// frame at that input request is the only moment their text can be observed.
// False when the test has not yet caused an input request.
boolean screenTextContains(const char *substring);

// Remember the background colour every cell was drawn with in the frame that
// screenTextContains() reads.  Call it while a transient highlight (the aim
// trajectory, say) is on screen, then compare later frames against it with
// screenCellColorsChanged().  Does nothing when no frame has been captured
// yet; screenCellColorsChanged() then reports false.
void rememberScreenColors(void);

// Whether the cell at `loc` is drawn with a different background colour now
// than when rememberScreenColors() took its snapshot - a highlight that was
// on screen then and has since been erased, for example.  The foreground is
// ignored because it is re-randomized on every draw; the background only
// jitters by a couple of colour points, so a blend of twenty points reads as
// a change.  False when either snapshot is missing or `loc` lies off the
// window.
boolean screenCellColorsChanged(pos loc);

// ---------------------------------------------
// Player actions (events)
// ---------------------------------------------

void restOneTurn(void);
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

// Perform a single melee attack from the given monster against the player,
// without ending the turn.
boolean monsterAttacksPlayer(creature *monst);

// Perform a single melee attack from *attacker* against *defender* (both can
// be any creature, including the player), without ending the turn.
boolean attackMonster(creature *attacker, creature *defender);

void playerZapsAt( item *theItem, creature *monst );

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
// Whether the player has ever seen the tile (it stays discovered once seen).
boolean cellIsDiscovered(pos loc);
boolean cellHasGas(pos loc);
boolean cellIsOnFire(pos loc);
// Whether any layer of the tile holds terrain that harms creatures standing
// on it: fire, damage, poison, paralysis, confusion, or explosive damage.
boolean cellIsHarmfulTerrain(pos loc);
boolean cellIsTrap(pos loc);
boolean cellIsKnownTrapFree(pos loc);
boolean isTrapTriggered(pos loc);
// Whether the tile belongs to the interior of a vault built during level
// generation.
boolean cellIsVaultInterior(pos loc);
// Whether the tile's liquid layer holds a liquid (water, lava, and so on).
boolean cellHasLiquid(pos loc);
// Whether the lighting pass last marked the tile as being in shadow.  Every
// tile starts each lighting recomputation in shadow; lights that dispel
// shadows clear the flag from the tiles they reach (and from their own
// origin tile), while the miner's light maintains it.
boolean cellIsInShadow(pos loc);

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
// Whether a hostile creature is currently holding the player's legs, as set
// by a seizing attack.
boolean playerIsSeized(void);
boolean isPlayerSubmerged(void);
boolean isPlayerPoisoned(void);
boolean isPlayerWeakened(void);
// Whether the player counts as standing in darkness: the local light on the
// tile they occupy is more than 10 below the colour of the miner's light.
boolean playerIsInDarkness(void);
boolean playerHasFeat(short feat);
boolean playerHasItem(short category, short kind);
// Mark a pack item as cursed, so that rules about curse handling (such as a
// scroll of enchanting removing a curse) can be set up deterministically.
void curseItem(item *theItem);
// Clear a pack item's curse, for rules whose setup needs a removable item.
void uncurseItem(item *theItem);
void removePackFood(void);
// Remove every pack item of the given category and kind, so a test can
// control exactly which items are available to be stolen or consumed.
void removePackItems(short category, short kind);

// Numeric status details, for rules about exact counters and amounts.
int playerStatusTurns(enum statusEffects status);
int playerWeaknessAmount(void);
int playerPoisonConcentration(void);
int playerShield(void);
int playerMovementSpeed(void);
int playerAttackSpeed(void);
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
// Apply a status that feeds into the derived equipment bonuses (the donning
// countdown, for example) and refresh those bonuses, so a test can read the
// result with playerDefense() without calling engine functions itself.
void applyEquipmentStatusToPlayer(enum statusEffects status, int duration);

// ---------------------------------------------
// Monsters (predicates and events)
// ---------------------------------------------

enum monsterTypes chooseMonsterTypeByProperty(enum boltType bolt, enum monsterBehaviorFlags behaviorFlagsInclude,  enum monsterBehaviorFlags behaviorFlagsExclude, enum monsterAbilityFlags abilityFlagsInclude, enum monsterAbilityFlags abilityFlagsExclude);
creature *spawnMonster(enum monsterTypes type, pos loc);
creature *findMonster(enum monsterTypes type);
void clearMonsters(void);
int monsterCount(void);
int countMonstersOfType(enum monsterTypes monstTyp);
// The nth monster on the level, in the engine's own creature order, or NULL
// when index is out of range.  Lets a test visit every monster without
// touching the engine's creature list.
creature *monsterAtIndex(int index);
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
boolean monsterIsDead(const creature *monst);
boolean monsterIsAlive(const creature *monst);
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
// The dungeon level the monster is standing on, which follows it across a
// level change only when the engine actually moves it.
int monsterDepth(const creature *monst);
boolean monsterIsAdjacentToPlayer(const creature *monst);
short vorpalEnemyClassForMonster(enum monsterTypes monstType);
short vorpalEnemyClassByName(const char *className);


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
int monsterAttackSpeed(const creature *monst);
void weakenMonster(creature *monst, int duration);

// The experience an ally has accumulated from discovered territory.
int monsterExperience(const creature *monst);

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
// Whether the monster's catalog entry gives it natural health regeneration.
boolean monsterRegenerates(const creature *monst);
// Chance, in percent, that the monster leaves its trail on the floor each
// turn (0 when it leaves no trail).
int monsterTrailChance(const creature *monst);
// Whether the trail the monster leaves is a puddle of urine, as the
// territory-marking animals leave behind.
boolean monsterLeavesUrineTrail(const creature *monst);
void poisonMonster(creature *monst, int duration, int concentration);
void slowMonster(creature *monst, int turns);
void hasteMonster(creature *monst, int turns);

void setMonsterSubmerged(creature *monst, boolean submerged);
void makeMonsterCaptive(creature *monst);
void makeMonsterDying(creature *monst);
void makeMonsterDormant(creature *monst);

boolean monsterIsHiddenFrom(const creature *monst, const creature *observer);
boolean monsterIsRevealedToPlayer(const creature *monst);
boolean monsterCouldAttack(const creature *attacker, const creature *defender);
boolean monstersAreAlliesOf(const creature *a, const creature *b);
boolean monstersAreFoesOf(const creature *a, const creature *b);
boolean monsterAvoidsCell(const creature *monst, pos loc);

// Whether the monster is carrying an item (a thief's stolen loot, for
// example).
boolean monsterIsCarryingItem(const creature *monst);

// Whether the monster is in the permanent flight that a steal-and-flee
// attack starts.
boolean monsterIsPermFleeing(const creature *monst);

// Give the monster an item to carry and put it into permanent flight, as it
// is after a successful steal-and-flee attack.
void makeMonsterStealAndFlee(creature *monst, short category, short kind);

// The monster loses whatever it was carrying, as a thief does when the item
// it stole is dropped or washed away.
void monsterLosesCarriedItem(creature *monst);

// Make the given monster a follower of the player.
void makeMonsterFollowPlayer(creature *monst);

// Tell an ally not to follow you any more: it holds its post and mills
// about nearby instead of tracking you, though it still chases enemies
// that come within its range.
void makeMonsterNotFollowPlayer(creature *monst);

// ---------------------------------------------
// Items (predicates and events)
// ---------------------------------------------

void removeFloorFood(void);
item *findInventoryItem(short category, short kind);
item *findFloorItem(short category, short kind);
item *findItem(short category, short kind);
item *givePlayerItem(short category, short kind, short enchant);
item *placeItemOnFloor(short category, short kind, pos loc);
// The tile an item occupies: where a floor item lies, or for a pack item the
// tile it was picked up from.
pos itemPosition(const item *theItem);
// How many ordinary (non-gold) items lie on the level.  Gold piles follow
// their own generation rule and are not counted.
int countFloorItems(void);
void equipItemFromPack(short category, short kind);
// Equip an item from the pack through the player's equip command, including
// the prompt shown when both ring slots are already full.  The prompt's answer
// comes from the test's queued input; a cancelled prompt leaves the equipment
// unchanged.
void equipItemFromPackAsPlayer(short category, short kind);
// Take off an equipped item of the given category and kind, so a test can
// cycle equipment one piece at a time.
void unequipItemFromPack(short category, short kind);

// The bonuses that wearing rings contributes, so a test can assert that a
// ring's enchantment applies without reading engine state.
int playerClairvoyanceBonus(void);
int playerStealthBonus(void);
int playerAwarenessBonus(void);
int playerWisdomBonus(void);
int playerRegenerationBonus(void);
int playerTransferenceBonus(void);
int playerLightMultiplier(void);
int playerReapingBonus(void);

// Drink a potion, applying its effect without ending the turn.
void drink(enum potionKind kind);
// Read a scroll, applying its effect (and ending the turn).
void readScrollOfKind(short kind);
// Use (apply) an item already in the pack: zap with a staff or activate a
// charm.  Any targeting prompt is answered from the queued keystrokes, or with
// escape when the queue is empty.
void applyPackItem(short category, short kind);

// Item details a rule can depend on.
int itemQuantity(const item *theItem);
int itemCharges(const item *theItem);
int itemEnchant(const item *theItem);
char itemInventoryLetter(const item *theItem);
short itemKind(const item *theItem);
short itemCategory(const item *theItem);
boolean itemIsCursed(const item *theItem);
boolean itemIsIdentified(const item *theItem);
boolean itemIsRunic(const item *theItem);
boolean runicIsIdentified(const item *theItem);
boolean runicIsHinted(const item *theItem);
unsigned long itemGoldValue(const item *theItem);
boolean itemKindIsKnown(short category, short kind);
// Whether magic detection has revealed this item's polarity, so the game
// knows whether its magic is good or bad (the state the inventory's glowing
// good/bad-magic symbol is drawn from).
boolean itemMagicWasDetected(const item *theItem);
// The polarity magic detection reveals: 1 good, -1 bad, 0 neutral.
int itemMagicPolarityValue(const item *theItem);
int runicActivationChance(const item *theItem);

// Key a runic item to a creature class: the class of creature that a slaying
// rune destroys, or the class of creature that an immunity armor protects
// against.  The class id comes from vorpalEnemyClassByName() or
// vorpalEnemyClassForMonster().
void setItemTargetClass(item *theItem, short classId);

// The items currently worn.
item *equippedWeapon(void);
item *equippedArmor(void);
// Whether an item is currently equipped.
boolean itemIsEquipped(const item *theItem);
// How many rings are currently on the player's fingers (0, 1, or 2).
int equippedRingCount(void);

// Counts within the pack.
int packStackCount(short category, short kind);
int packUnitCount(short category, short kind);
int packItemCount(void);
// Fill the pack to its maximum capacity, for rules that depend on a full pack.
void fillPackToCapacity(void);

// Player numbers related to items.
int playerGold(void);
int playerStrength(void);
int playerDefense(void);

// Damage the player through the engine's own damage routine, so rules about
// damage reduction (such as easy mode) apply.
void woundPlayer(int amount);

// Succumb to easy mode.
void enterEasyMode(void);

// ---------------------------------------------
// Game start-up
// ---------------------------------------------

// Start the game over from the beginning with the same seed, running the
// whole start-up sequence again exactly as the game does when a run begins:
// the message log and archive are cleared, the screen is blacked out, and a
// freshly generated dungeon welcomes the player.
void restartGame(void);

// ---------------------------------------------
// Main menu (variant dialog)
// ---------------------------------------------

// The game variant currently in effect, named as the Change Variant dialog
// names it: "Brogue", "Rapid Brogue", or "Bullet Brogue".
const char *gameVariantName(void);

// Show a one-button dialog whose label is wider than the screen, centred so
// that both ends of the label land off the display window, and report whether
// Return chose the button.  The dialog answers from the test's queued input
// (a Return is queued here), and is dismissed again before returning, so the
// drawn label can be read afterwards with screenTextContains().
boolean showOversizedButton(const char *label);

// Show the main menu's Change Variant dialog and answer it from the test's
// queued input: `answerKey` is one of the dialog's own letter hotkeys, or
// ESCAPE_KEY to cancel the dialog.  The menu is dismissed again before
// returning, so the outcome can be read with gameVariantName().
void showVariantDialog(int answerKey);

// Show the main menu's Change Mode dialog and answer it from the test's
// queued input: `answerKey` is one of the dialog's own letter hotkeys, or
// ESCAPE_KEY to cancel the dialog (leaving the mode in effect unchanged).
// The menu is dismissed again before returning; in test mode the dialog's
// full text is recorded, so its wording can be read with messageWasShown().
void showModeDialog(int answerKey);

// Show the main menu's title screen — the screen offering New Game, Play,
// View and Quit — and dismiss it again by answering Quit.  The menu is
// dismissed before returning, so the drawn entries can be read afterwards
// with screenTextContains().
void showTitleScreen(void);

// ---------------------------------------------
// Menu buttons
// ---------------------------------------------

// Draw one menu button at the given opacity over a solid white screen and
// report how much of that screen still shows through the button, in percent:
// 0 when the button is fully opaque, 100 when it is fully transparent.  The
// screen is restored before returning.
int buttonBackdropVisibility(int opacity);

// Open the real inventory over the dungeon, dismiss it, and report the
// opacity its buttons were painted with: 0 is fully transparent and 100 fully
// opaque.  Returns -1 when no button was drawn or -2 when the buttons
// disagree.
int inventoryButtonOpacity(void);

// Report the opacity the bottom-row buttons were painted with in the frame
// the game drew just now: 0 is fully transparent and 100 fully opaque.  As
// with inventoryButtonOpacity(), -1 means no button was drawn and -2 means
// the buttons disagree.
int bottomRowButtonOpacity(void);

// Paint one ordinary menu button through the game's own button painter,
// without any surrounding menu, so the randomness it consumes can be
// observed directly.
void paintMenuButton(const char *label);

// ---------------------------------------------
// Random-number sequence
// ---------------------------------------------

// How many numbers the game has drawn from the sequence that governs
// gameplay.  Painting menus and other decorative work must not change it.
unsigned long gameplayRandomDraws(void);

// Whether the game's own random-number sequence is the one currently in
// effect (rather than the decorative one used while drawing).
boolean gameplayRandomStreamActive(void);

// Draw one number from the game's own random sequence, for use as a control
// that the sequence and its counter are observable.
int drawGameplayRandomNumber(void);

// Framework internal: remember the opacity the button painter stored in one
// cell of a menu's off-screen layer.  Called by the painter while testing.
void testRecordButtonCellOpacity(short x, short y, short opacity);

// ---------------------------------------------
// Game modes and high scores
// ---------------------------------------------

// Open the wizard mode dialog that creates an item or a monster.  While the
// game stays in test mode the dialog answers from the test's queued
// keystrokes, so no screen or real input is needed.
void attemptWizardItemCreation(void);

// Enter wizard mode: a victory in wizard mode is never scored.  Wizard mode
// turns off the test-mode input plumbing, so a test that enters it must also
// avoid every prompt: use enableServerMode() and enableAutopilot().
void enterWizardMode(void);

// Enter playback mode: the game replays a recording, skips all prompts, and
// never saves a score or a run history entry.
void enterPlaybackMode(void);

// Enter playback armed with a recording of `turns` out-of-sync checks plus
// room to advance `turns` further turns.  The checks are recorded from two
// real passes over alternating UPRIGHT/DOWNLEFT steps (so the replay
// reproduces the recording pass's per-turn randomness), and periodic
// environment updates are pushed beyond all passes.  In test mode a playback
// seek takes its events from the test's queued input while the engine's
// per-turn out-of-sync check still reads the recording, so queueing the same
// steps afterwards makes an "advance N turns" command observable with
// playerTurnCount().
void startPlaybackWithRecordedChecks(unsigned int turns);

// Run as a server: the victory path skips the save-recording prompt and the
// high-score screen, so the game never asks for input.
void enableServerMode(void);

// Put the given amount of gold in the player's purse.
void givePlayerGold(int amount);

// The score a victory recorded right now would carry: gold plus the value of
// every carried item.
int expectedVictoryScore(void);

// Whether the high-score file currently holds an entry with this score.
boolean highScoreHasScore(int score);

// The description saved in the high-score entry carrying this score, or the
// empty string when no entry has it.
const char *highScoreDescription(int score);

// Empty the high-score list before a run whose recorded score a test will
// assert on: saveHighScore() refuses to write below the list's lowest
// entry, so a fresh test needs a known, empty list to write into, and an
// entry found afterwards was written by that run alone.
void clearHighScores(void);

// Wear a freshly generated suit of armor at a chosen enchantment.
void wearArmorOfEnchant(short kind, int enchant);

// Create a freshly generated plain (non-runic, uncursed) weapon in the
// player's pack at an exact enchantment: generateItem() otherwise rolls a
// random enchant and possibly a runic.
item *givePlayerPlainWeapon(short kind, int enchant);

// Create a freshly generated runic weapon/armor in the player's pack at the
// chosen enchantment, already identified as runic.
item *givePlayerRunicWeapon(short kind, short runicKind, short enchant, boolean revealIdentity);
item *givePlayerRunicArmor(short kind, short runicKind, short enchant, boolean revealIdentity);

// ---------------------------------------------
// Dungeon (predicates and events)
// ---------------------------------------------

// The fixed dimensions of a dungeon level, in tiles.
int levelWidth(void);
int levelHeight(void);

// How many tiles of the dungeon layer hold the given terrain.
int countTerrain(short terrainType);

// ---------------------------------------------
// Vault layouts
// ---------------------------------------------

// Reward vaults choose among their layouts by a weighted raffle.  A layout is
// an opaque index in [0, vaultLayoutCount()), and every layout shown here is a
// reward vault that can enter that raffle.  drawVaultLayout() performs the
// engine's own choice for the given depth, so a test can sample it without
// touching the blueprint catalog.
int vaultLayoutCount(void);
int vaultLayoutMinDepth(int layout);
int vaultLayoutMaxDepth(int layout);
int vaultLayoutFrequency(int layout);
int drawVaultLayout(int depth);

// Descend one level, generating the next dungeon level.
void descendToNextLevel(void);

// Descend to the amulet's floor with the amulet altar machine rigged to fail
// every build attempt, so that the engine's fallback amulet placement is the
// only way an amulet can appear.  Returns once the amulet floor is built.
void descendWithFailedAmuletAltar(void);

// Climb one level, the mirror of descendToNextLevel.
void ascendToPreviousLevel(void);

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

// The colour the player is currently drawn with: "invisible", "darkness",
// "shadow", "light", or "unknown".  The four named colours are distinct.
const char *playerDrawColorName(void);
// An opaque id of the current draw colour, built from its RGB components, so
// a test can compare colours without reading engine state.  Equal ids mean
// equal colours.
unsigned long playerDrawColorId(void);
// Recompute the player's draw colour from the current light map without
// repainting the lighting, so a test can inspect the darkness colour.
void updatePlayerDrawColor(void);

// How many floor tiles of the level are cut off from the region containing
// `from`.  Passability follows the rule the generator flood-fills with when
// it decides whether a placement would strand part of the level: ordinary
// walls block, while doorways, secret doors, portcullises, barricades and
// statue doorways count as connections.
int countUnreachableFloorFrom(pos from);

// ---------------------------------------------
// World (predicates and events)
// ---------------------------------------------

unsigned long turnNumber(void);
int ticksUntilPlayerActs(void);
// Ticks remaining before the environment next updates; the player's haste and
// slow expiry resynchronize this counter with the player's time state.
int ticksUntilEnvironmentUpdate(void);
int currentDepth(void);
void setDepth(int depth);
boolean isSameDepth(void);
boolean playerIsCarryingAmulet(void);
int deepestDepth(void);
int totalGoldGenerated(void);
int playerTurnCount(void);

// ---------------------------------------------
// TEST SETUP HELPERS
// ---------------------------------------------

void clearArena(void);
void setArenaRadius(int radius, enum tileType tile);
item *giveAndEquipRunicWeapon(short kind, short runicKind, short enchant, boolean revealIdentity);
creature *placeHostileMonster(enum monsterTypes type, pos loc);

// Spawn a monster through the same generation path level population uses,
// letting it draw a random item from the level's monster-item hopper when its
// catalog entry says it carries one (always for MONST_CARRY_ITEM_100, a
// quarter of the time for MONST_CARRY_ITEM_25).  spawnMonster() switches item
// generation off, so a monster spawned with it never holds anything.
creature *spawnMonsterWithItemChance(enum monsterTypes type, pos loc);

// Add a dead ally of the given kind to purgatory, so it can be raised with
// resurrectDeadAllyAt.  The ally dies at `loc` and awaits resurrection there.
void addDeadAllyToPurgatory(enum monsterTypes type, pos loc);

// Raise the most empowered dead ally at the given location.  Returns true if
// an ally was raised; it is placed on nearby ground free of harmful terrain
// and occupants.
boolean resurrectDeadAllyAt(pos loc);

// ---------------------------------------------
// TEST STEP HELPERS
// ---------------------------------------------

// Feed the queued input through the engine's main input loop — the loop a live
// game pumps for player input — and return as soon as the queue runs dry.  The
// loop has no other exit, so the input hook flags the game as ended when the
// queue empties; this helper clears that flag before returning and discards
// anything left queued.
void runQueuedInputThroughMainLoop(void);

boolean wait_until_fall_step(void);
boolean wait_until_hurt(void);
boolean trap_step(void);
boolean wait_10_turns_step(void);
boolean wait_100_turns_step(void);
boolean move_right_step(void);


// ---------------------------------------------
// TEST VERIFY HELPERS
// ---------------------------------------------

boolean player_is_hurt_verify(void);

#endif
