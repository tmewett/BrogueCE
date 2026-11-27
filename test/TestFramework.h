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
// Supply a mouse click outside the message area: in the sidebar column below
// the messages, away from the map and the bottom-row buttons.
void queueClickOutsideMessageArea(void);
// Supply a mouse click on a dungeon tile, moving the targeting cursor there.
// The engine confirms a target only when the cursor is already on the clicked
// tile, so aiming at a tile takes two of these: the first moves the cursor and
// the second confirms it.
void queueMapClick(pos loc);
void queueConfirmYes(void);
void queueConfirmNo(void);

// Show a two-button confirmation prompt and report whether it was accepted.
boolean askConfirmation(void);

// Open the engine's file-picker dialog (the one the main menu uses for saved
// games and recordings) with the given filename suffix, and report whether it
// selected a file.  The dialog's answers come from the test's queued input.
boolean chooseFileWithSuffix(const char *suffix);

// Open the game's saving prompt and report the default file name it offers, or
// the empty string when it offers none.  The prompt is cancelled again with
// Escape, so no file is written.
const char *defaultSaveName(void);

// Framework internal: remember the text a text-entry prompt started with.
void recordTestTextInputDefault(const char *text);

// Turn autopilot on (confirmations are then skipped and treated as "yes").
void enableAutopilot(void);

// The game version string, as the game reports it.
const char *gameVersionString(void);

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
// Engine-facing: note whether the engine asked the player to acknowledge the
// message just recorded.  Test mode suppresses the real --MORE-- prompt, so
// this remembers the flag for messageRequiredAcknowledgment().
void recordTestMessageAcknowledgment(const char *msg, boolean requiresAck);
void clearMessages(void);
// Show any combat messages buffered since the last turn, so a test can assert
// on what a blow or an exposure said without ending a turn.
void flushCombatMessages(void);
// Turn on cautious mode: the setting under which the game announces each
// monster it spots with an acknowledged message ("you see a goblin").  Test
// mode never waits at the --MORE-- prompt, so the mode survives the
// announcement, but flushing combat messages turns it off again, so a test
// should enable it before each turn whose announcement it wants to observe.
void enableCautiousMode(void);
// Whether any message has been shown since the log was last cleared.
boolean anyMessageWasShown(void);
boolean messageWasShown(const char *substring);
// Whether the recorded message containing `substring` was sent with the
// engine's require-acknowledgment flag, i.e. a real game would have stopped at
// a --MORE-- prompt for it.  False when no such message was recorded.
boolean messageRequiredAcknowledgment(const char *substring);
const char *lastMessage(void);
void printTestMessages(void);

// Temporary alerts are painted straight to the screen and erased again, so
// they never reach the message log or a screen snapshot.  These record the
// text at the moment the engine flashes an alert, and let a test ask what was
// flashed during the current test.
// Engine-facing: remember one flash alert.  Called from flashTemporaryAlert().
void recordTestFlashAlert(const char *message);
void clearFlashAlerts(void);
boolean flashAlertWasShown(const char *substring);
// The text of the most recently flashed alert, or the empty string.
const char *lastFlashAlert(void);

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

// How many messages the archive can hold before the oldest is overwritten.
int messageArchiveCapacity(void);
// How many archive slots currently hold a message (at most the capacity).
int messageArchiveEntryCount(void);
// The text of the archived message `back` entries back, where 1 is the newest.
// Empty string when `back` is out of range or the slot is empty.
const char *messageArchiveEntry(int back);

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

// Brightness of the message-area text containing `substring`, in the game's
// 0-100 colour scale, or -1 when no displayed message line contains it.
// Unconfirmed messages are drawn at their full brightness; confirmed ones are
// dimmed.
int messageAreaBrightness(const char *substring);

// ---------------------------------------------
// Screen
// ---------------------------------------------

// Redraw the sidebar and report whether the given text appears in it.  The
// sidebar is the stats column at the left edge of the screen, so a match here
// can only have come from the sidebar.
boolean sidebarContains(const char *substring);
// Redraw the sidebar with the examine cursor resting on `loc` and report the
// brightness (0-100) of the foreground colour of the first row containing
// `substring`, or -1 when no row contains it.  The entity under the cursor is
// drawn undimmed while the other listed entities are dimmed.
int sidebarTextBrightnessWithFocus(pos loc, const char *substring);

// The flavor-text line at the bottom of the map, exactly as the game currently
// draws it (capitalized as the player sees it, without trailing blanks), or the
// empty string when that line is blank.
const char *flavorTextLine(void);

// Redraw the flavor-text line through the game's own refresh path, as happens
// after a turn, a move, or closing a screen.
void refreshFlavorText(void);

// The game's full description of a map square, as shown in the flavor line
// when the player looks at it (the player's own story on their tile, a
// creature, an item, or the terrain).  Reads the current game state without
// drawing anything.
const char *locationDescriptionAt(pos loc);

// Report whether the given text appears on the whole screen as of the most
// recent time the engine asked the test for input.  Full-screen displays (the
// discoveries screen, the message archive, the help screen) draw themselves
// over the map, wait for a key, and then restore the previous screen, so the
// frame at that input request is the only moment their text can be observed.
// False when the test has not yet caused an input request.
boolean screenTextContains(const char *substring);

// The line of that same frame containing `substring`, or the empty string
// when the frame is missing or no line contains it.  A line keeps its columns
// intact, so a test can tell one value spanning several columns from several
// values in separate columns.
const char *screenTextLineContaining(const char *substring);

// The row (0 is the top row of the screen) of that same frame containing
// `substring`, or -1 when the frame is missing or no row contains it.
int screenTextRowContaining(const char *substring);

// The column (0 is the left edge of the screen) at which `substring` begins in
// that same frame, or -1.  This is the first occurrence in the topmost row
// that contains it.
int screenTextColumnContaining(const char *substring);

// The top row and leftmost column of the dungeon display window, the area the
// game draws the map and its overlays into (right of the sidebar).
int displayWindowTopRow(void);
int displayWindowLeftColumn(void);

// Open the help screen over the current display, dismiss it, and report
// whether the display afterwards is exactly the one shown before it opened.
// helpScreenWasShown() reports whether its command list was actually drawn.
boolean helpScreenRestoresPreviousDisplay(void);
boolean helpScreenWasShown(void);

// In the discovered-items screen, the mark drawn beside a kind listed by
// `name`: 1 for the good-magic marker, -1 for the bad-magic marker, and 0 when
// the kind is identified (shown by its item glyph) or is not on screen.
int discoveryScreenMagicMarkerFor(const char *name);

// Open the real inventory over the dungeon, dismiss it, and report the magic
// mark drawn beside the pack item with the given inventory letter, once magic
// detection has revealed the item's polarity: 1 for the glowing good-magic
// mark, -1 for the glowing bad-magic mark, and 0 for the plain mark neutral
// magic gets.  Returns -2 when the item's line carries no magic mark at all
// (before detection, for example) or is not on screen.
int inventoryScreenMagicMarkerFor(char inventoryLetter);

// Open the real inventory, press `openKey` to open the item detail box, press
// `pageKey` `pages` times to page through the details, then dismiss it.  The
// detail text shown last is returned.  The up and down arrows are the
// keyboard-only controls the inventory and its detail box each add for
// paging; this exercises them without exposing the menu's buttons.
const char *inventoryDetailAfterKeys(int openKey, int pageKey, int pages);

// The same as inventoryDetailAfterKeys(), but the frame the detail box draws
// is held so screenTextContains() can read its buttons afterwards.  The detail
// text shown last is returned.
const char *inventoryDetailHeldAfterKeys(int openKey, int pageKey, int pages);

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
// Run in the given direction: keep stepping until the engine decides the run
// is disturbed.  The whole run happens synchronously.
void runDirection(enum directions dir);
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
// Zap a staff at a monster exactly as using the staff does, including the
// staff's enchantment-scaled bolt magnitude, but without spending a turn.
void playerZapsStaffAt( item *theItem, creature *monst );

// ---------------------------------------------
// Terrain (events and predicates)
// ---------------------------------------------

void clearTerrain(pos loc);
void setTerrain(pos loc, short terrainType);
void setTerrainAroundLoc(pos loc, short terrainType);
// Place a glowing glass pipe on `loc`, wired into a fresh mechanism, so a test
// can observe what powering that mechanism does to it.
void placeGlowingGlassPipeWiredToMechanism(pos loc);
// Power the mechanism that includes the tile at `loc`, through the engine's
// own machine-activation routine.  Does nothing when the tile is not part of a
// mechanism.
void powerMechanismIncluding(pos loc);
// Place a crystal globe on `globeLoc` and a glowing glass pipe on `pipeLoc`,
// wiring both into the same fresh mechanism.  A shining globe is placed when
// `shining` is true; otherwise the globe is darkened.  The globe's state
// decides whether the mechanism can activate.
void placeGlobeAndPipeWiredToMechanism(pos globeLoc, pos pipeLoc, boolean shining);
// Expose the terrain at `loc` to electricity, as a lightning bolt does.
// Returns whether anything was promoted.
boolean exposeGlobeToElectricShock(pos loc);
// Place a stone archway portal on `loc`, wired into a fresh mechanism.
void placePortalWiredToMechanism(pos loc);
// Whether the tile holds a glowing glass pipe.
boolean cellIsGlowingGlassPipe(pos loc);
// Whether the tile holds a charred (inert) glass pipe.
boolean cellIsCharredGlassPipe(pos loc);
void spawnFeature(pos loc, short feature);
void revealCell(pos loc);
void placeDownStairs(pos loc);
void placeUpStairs(pos loc);
// The square holding the current level's down staircase and up staircase.
pos downStairsLocation(void);
pos upStairsLocation(void);
// The staircase squares the dungeon reserved for the given depth when its
// layout was drawn, before that level has been built.  Depth is one-based and
// must be at least 3 so the reserved values have not yet been replaced by the
// built level's refined staircase positions.  Returns false when the depth is
// out of range or already built.
boolean dungeonLevelStairLocations(int depth, pos *upstairs, pos *downstairs);
// The distance between the reserved staircases of the given depth, measured
// the way the game measures distance: the larger of the horizontal and
// vertical gaps.  Returns -1 when no reserved layout is available.
int dungeonLevelStairSeparation(int depth);
short terrainAt(pos loc);
boolean cellIsTerrain(pos loc, short terrainType);
boolean cellObstructsPassage(pos loc);
boolean cellIsVisible(pos loc);
// Whether the tile lies in the player's unobstructed line of sight, whether
// or not there is enough light to see what is on it.
boolean cellIsInFieldOfView(pos loc);
// Whether the player has ever seen the tile (it stays discovered once seen).
boolean cellIsDiscovered(pos loc);
boolean cellHasGas(pos loc);
// Put a gas of the given kind on a tile with an exact volume, replacing any
// gas already there.  The tile's terrain is untouched, so a test can strand
// gas on terrain that cannot hold it.
void setGasAt(pos loc, short gasType, int volume);
int gasVolumeAt(pos loc);
short gasTypeAt(pos loc);
// Gas kinds chosen by their dissipation behavior: one that dissipates
// quickly, one that dissipates slowly, and one that never dissipates.
short quickDissipatingGasType(void);
short slowDissipatingGasType(void);
short nonDissipatingGasType(void);
// Run one periodic environment update (the pass that moves gas, spreads
// fire, and so on) without advancing the turn count.
void runEnvironmentUpdate(void);
// Run a single gas-simulation pass, without the rest of an environment
// update and without advancing the turn count.
void runGasUpdatePass(void);
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
// Whether the tile is a corridor tile: passable ground pinched between walls
// (or a corridor junction), as opposed to the open floor of a room.
boolean cellIsHallway(pos loc);
// Whether the tile's liquid layer holds a liquid (water, lava, and so on).
boolean cellHasLiquid(pos loc);
// Whether the lighting pass last marked the tile as being in shadow.  Every
// tile starts each lighting recomputation in shadow; lights that dispel
// shadows clear the flag from the tiles they reach (and from their own
// origin tile), while the miner's light maintains it.
boolean cellIsInShadow(pos loc);

// The colour a terrain tile is drawn with, named for the tile's role:
// "torch orange" for the flame of an ordinary or not-yet-triggered wired wall
// torch, "haunted torch purple" for a sputtering torch, and "other" for any
// other tile.  Equal names mean equal colours.
const char *terrainDrawColorName(pos loc);
// Whether the terrain tile's drawn colour re-shades itself on every screen
// refresh (its colour "dances").
boolean terrainDrawColorFlickers(pos loc);
// The colour of the light the terrain tile casts, named the same way as the
// drawn colour ("torch orange" for torches, candles, altars and pedestals,
// "haunted torch purple" for a sputtering torch, "other" otherwise).  Equal
// names mean equal light colours.
const char *terrainGlowLightColorName(pos loc);

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
// Whether the run itself has ended (the player died, won or quit), as
// opposed to the player merely reaching zero health.
boolean runHasEnded(void);
// The cause the engine recorded for the player's death, exactly as gameOver()
// received it ("spark turret", "a fall", "Drained by a cursed ring") and
// before the rendered "Killed by ... on depth N" sentence is built.  Empty
// while the player has not died.
const char *playerDeathCause(void);
// Whether the player's death was attributed to exactly `cause`, e.g. the
// monster whose bolt landed the killing blow.
boolean playerDeathWasAttributedTo(const char *cause);
// Engine-facing: remember the cause passed to gameOver().
void recordPlayerDeathCause(const char *cause);
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
// Remove every item from the pack, leaving the player carrying nothing.
void clearPack(void);

// Numeric status details, for rules about exact counters and amounts.
int playerStatusTurns(enum statusEffects status);
int playerWeaknessAmount(void);
int playerPoisonConcentration(void);
int playerShield(void);
int playerMovementSpeed(void);
int playerAttackSpeed(void);
int playerArmorValue(void);
// The player's current stealth range: how close a creature must come before
// it notices the player.  Reflects lighting, armor, rings and status effects.
int playerStealthRange(void);

// Apply a status effect through the engine's own routines.
void weakenPlayer(int duration);
void poisonPlayer(int duration, int concentration);
void slowPlayer(int turns);
void hastePlayer(int turns);
void healPlayerPanacea(void);
void applyTileEffectsToPlayer(void);
// Expose the player to fire through the engine's own catch-fire routine, so a
// test can observe whether the flame sets the player alight and what it says.
void exposePlayerToFire(void);

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
// Spawn a horde at `loc`, giving it `followers` members so both its leader
// and its member path are exercised.  `allied` makes the horde one of the
// hordes flagged as allied with you.  Returns the horde's leader;
// *membersSpawned receives the number of followers actually placed (pass NULL
// when not needed).
creature *spawnHordeWithFollowers(pos loc, int followers, boolean allied, int *membersSpawned);
// The allied form used by the horde-allegiance rule.
creature *spawnAlliedHordeWithFollowers(pos loc, int followers, int *membersSpawned);
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
// Whether a wandering monster's current destination is the part of the level
// nearest `destination`: the checkpoint the game picks when it sends a
// monster heading for a remembered location.  False when the monster is not
// wandering or has no destination.
boolean monsterWanderDestinationIsNearestTo(const creature *monst, pos destination);
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
// Remove every monster from the level above yours, so only the ones a test
// deliberately waits there can enter.
void clearMonstersOnPreviousLevel(void);
// Move a monster onto the level above yours, as though it were following you,
// and have it reach your level after `updates` 100-tick environment updates.
// The monster must be on your level when this is called; the previous level
// must already have been visited.
void waitOnPreviousLevelToEnter(creature *monst, int updates);
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

// Whether `follower` follows `leader`: it carries the follower status and its
// leader link points at `leader`.  False when either pointer is NULL or when
// the same monster is passed as both.
boolean monsterFollows(const creature *follower, const creature *leader);

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
// Whether the monster is carrying another monster of the given kind inside
// it, as a summoner that enters its summons does.
boolean monsterCarriesMonsterOfType(const creature *monst, enum monsterTypes type);
// Give the monster the chance to use its summon ability through the engine's
// own routine, retrying the engine's random roll until a summon succeeds or
// the attempt cap is reached.  Returns false when the monster cannot summon.
boolean forceMonsterSummon(creature *monst);
// Remove monsters that have already died from the level, so a test can count
// what is left without also counting corpses.
void clearDeadMonsters(void);

// Base stat predicates (from monsterCatalog info).
int monsterMaxHealth(const creature *monst);
int monsterBaseDefense(const creature *monst);
int monsterBaseAccuracy(const creature *monst);
int monsterBaseDamageLow(const creature *monst);
int monsterBaseDamageHigh(const creature *monst);
int monsterBaseMovementSpeed(const creature *monst);
int monsterBaseAttackSpeed(const creature *monst);
// Override the monster's catalog accuracy, so a test can make a blow that
// cannot land normally while leaving the auto-hit paths intact.
void setMonsterAccuracy(creature *monst, int accuracy);
// Whether the monster's catalog entry marks it as large.
boolean monsterIsLarge(const creature *monst);
// Whether the monster's catalog entry gives it natural health regeneration.
boolean monsterRegenerates(const creature *monst);
// The gender the engine assigned the monster: "male", "female", or "neuter".
// A kind that can be either male or female is given exactly one of the two
// when it is generated; a kind that names neither is left neuter.
const char *monsterGenderName(const creature *monst);
// Chance, in percent, that the monster leaves its trail on the floor each
// turn (0 when it leaves no trail).
int monsterTrailChance(const creature *monst);
// Whether the trail the monster leaves is a puddle of urine, as the
// territory-marking animals leave behind.
boolean monsterLeavesUrineTrail(const creature *monst);
// Whether the trail the monster leaves is a droplet of ectoplasm, as the
// phantom's is.
boolean monsterLeavesEctoplasmDroplet(const creature *monst);
// The colour the monster is drawn with, named for the monster's role:
// "wisp blue" for a will-o'-the-wisp, "pink" for a pink jelly, "tan" for an
// underworm, "dark blue" for a sentinel, "blue" for a goblin mystic,
// "purple" for an ifrit, "red" for a phoenix or its egg, "other" for any
// other monster.  Equal names mean equal colours.
const char *monsterDrawColorName(const creature *monst);
// Whether the monster's drawn colour re-shades itself on every screen refresh
// (its colour "dances").
boolean monsterDrawColorFlickers(const creature *monst);
// The colour of the light the monster casts on its own, named the same way as
// the drawn colour ("wisp blue" for a will-o'-the-wisp, "other" when the
// monster casts no light of its own).  Equal names mean equal light colours.
const char *monsterGlowLightColorName(const creature *monst);
// The reach of the light the monster casts on its own, in tiles (its
// smallest reach when the radius varies); 0 when the monster casts no light.
double monsterGlowLightRadius(const creature *monst);
// Whether the monster's own light changes its shade or reach on every screen
// refresh (its light "flickers").
boolean monsterGlowLightFlickers(const creature *monst);
// The base red, green and blue components of the light the monster casts on
// its own; all zero when the monster casts no light.
void monsterGlowLightColorComponents(const creature *monst, int *red, int *green, int *blue);
void poisonMonster(creature *monst, int duration, int concentration);
void slowMonster(creature *monst, int turns);
void hasteMonster(creature *monst, int turns);

void setMonsterSubmerged(creature *monst, boolean submerged);
// Whether the monster is hidden beneath the surface of deep water.
boolean monsterIsSubmerged(const creature *monst);
// Expose a monster to fire through the engine's own catch-fire routine, so a
// test can observe whether the flame sets it alight.
void exposeMonsterToFire(creature *monst);
void makeMonsterCaptive(creature *monst);
void makeMonsterDying(creature *monst);
void makeMonsterDormant(creature *monst);

boolean monsterIsHiddenFrom(const creature *monst, const creature *observer);
boolean monsterIsRevealedToPlayer(const creature *monst);
// Whether the monster's own glyph is currently drawn on the map at its
// location, as opposed to the tile underneath it.
boolean monsterIsShownOnMap(const creature *monst);
boolean monsterCouldAttack(const creature *attacker, const creature *defender);
boolean monstersAreAlliesOf(const creature *a, const creature *b);
boolean monstersAreFoesOf(const creature *a, const creature *b);
boolean monsterAvoidsCell(const creature *monst, pos loc);

// Whether the monster is carrying an item (a thief's stolen loot, for
// example).
boolean monsterIsCarryingItem(const creature *monst);

// The item the monster is carrying, or NULL when it carries nothing.
item *monsterCarriedItem(const creature *monst);

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
// Set the cumulative nutrition the dungeon's ordinary item generation has
// provided so far this game, so a test can put the dungeon behind or ahead of
// the food the current depth is expected to have supplied.
void setFoodNutritionGenerated(int nutrition);
// Remove every item from the floor, leaving the terrain untouched.
void clearFloorItems(void);
item *findInventoryItem(short category, short kind);
item *findFloorItem(short category, short kind);
item *findItem(short category, short kind);
item *givePlayerItem(short category, short kind, short enchant);
// Give the player an item whose kind has not been identified, so a test can
// observe the behavior shown only before the player learns what it is.
item *givePlayerUnidentifiedItem(short category, short kind, short enchant);
// Mark that the player has learned this item's maximum charge count, without
// identifying the item itself.
void markItemMaxChargesKnown(item *theItem);
item *placeItemOnFloor(short category, short kind, pos loc);
// The tile an item occupies: where a floor item lies, or for a pack item the
// tile it was picked up from.
pos itemPosition(const item *theItem);
// The dungeon depth the item was generated on, or the depth it was handed to
// its owner on when it came out of a shared pool.  Zero when it was never
// tagged.
int itemOriginDepth(const item *theItem);
// How many ordinary (non-gold) items lie on the level.  Gold piles follow
// their own generation rule and are not counted.
int countFloorItems(void);
// The nth ordinary (non-gold) item on the floor, in the engine's own order,
// or NULL when index is out of range.
item *floorItemAtIndex(int index);
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
// Read a scroll through the engine's own scroll routine without ending the
// turn, so an immediate effect can be observed before any status counter
// ticks down in the turn that follows.
void readScrollOfKindWithoutEndingTurn(short kind);
// Use (apply) an item already in the pack: zap with a staff or activate a
// charm.  Any targeting prompt is answered from the queued keystrokes, or with
// escape when the queue is empty.
void applyPackItem(short category, short kind);

// Item details a rule can depend on.
int itemQuantity(const item *theItem);
int itemCharges(const item *theItem);
// Overwrite how many charges (uses) a staff or wand has left.
void setItemCharges(item *theItem, int charges);
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
// The game's full examine description of an item, exactly as its item box
// prints it, including the paragraphs about a runic effect.  Reads the
// current game state without drawing anything.
const char *itemExamineText(const item *theItem);

// The separate fields the seed catalog's CSV writer fills for one item.  Each
// string is exactly the column the engine writes, taken from a single CSV line
// produced by the catalog's own item-reporting code.
typedef struct {
    char category[32];
    char kind[64];
    char enchantment[16];
    char runic[64];
} seedCatalogItemFields;

// Write the item as one seed-catalog CSV line with the engine's writer and
// split it into its category, kind, enchantment and runic columns.  Returns
// false (and sets testFailed) if no well-formed line was produced.
boolean seedCatalogCsvFieldsForItem(item *theItem, seedCatalogItemFields *out);

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

// Resolve an incoming blow through the armor-rune handler by the route a
// thrown weapon that hits you takes (the throw path calls it with the melee
// flag clear), and return the damage left after any rune effect.  `thrower` is
// the creature that threw the weapon.
int damageAfterArmorRuneForThrownWeapon(creature *thrower, int incomingDamage);

// Succumb to easy mode.
void enterEasyMode(void);
// Enable easy mode through the game's own in-game command, confirming its
// prompt, so the announcement it prints can be observed.
void enableEasyModeThroughCommand(void);

// ---------------------------------------------
// Game start-up
// ---------------------------------------------

// Start the game over from the beginning with the same seed, running the
// whole start-up sequence again exactly as the game does when a run begins:
// the message log and archive are cleared, the screen is blacked out, and a
// freshly generated dungeon welcomes the player.
void restartGame(void);

// Start a fresh game with the given master seed, running the whole start-up
// sequence again exactly as a new run does.  Every level's seed and stair
// placement is regenerated before the first level is built.
void startGameWithSeed(unsigned long long seed);

// Copy the seed reserved for each level of the dungeon, from its first level
// through the deepest, into `seeds` and return how many values were copied.
// At most `capacity` values are written.
int dungeonLevelSeeds(unsigned long long *seeds, int capacity);

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

// Open the main menu's New Seeded Game seed prompt, type `digits`, and accept
// it, starting a new game with the seed those digits denote.  The prompt's
// entry starts empty regardless of the previous game's seed, so the accepted
// seed is exactly what the entry kept.  The new game's input loop is ended as
// soon as it asks for input, so the call returns with the game's seed readable
// through dungeonSeed().  The prompt frame is held, so its wording can be read
// afterwards with screenTextContains().
void startSeededGameByTyping(const char *digits);

// Show the main menu's title screen — the screen offering New Game, Play,
// View and Quit — and dismiss it again by answering Quit.  The menu is
// dismissed before returning, so the drawn entries can be read afterwards
// with screenTextContains().
void showTitleScreen(void);

// Run the real title screen for `passes` input-loop passes while the game
// reports its window active, then quit.  Returns true only when a frame was
// drawn on every pass and each pass's frame differed from the one before it.
// A title screen that draws once and then leaves the display alone, or that
// does not draw at all, fails the check.
boolean titleScreenRedrawsOnEveryActivePass(int passes);

// Run the real title screen for one active pass, then `inactivePasses`
// input-loop passes while the game reports its window inactive, then quit.
// Returns true only when the title screen was drawn during the active pass
// and the display afterwards is exactly the frame that had already been
// drawn, with no new flame frame or menu redraw during the inactive passes.
boolean titleScreenLeavesDisplayAloneWhileApplicationInactive(int inactivePasses);

// Show the main menu's View flyout — the menu opened by pressing View — and
// dismiss it again by answering Quit.  The flyout frame is held, so its
// entries can be read afterwards with screenTextContains().
void showViewFlyout(void);

// Open the View flyout, choose View Recording with its hotkey, and cancel the
// recording picker.  A short recording is saved first so the picker has a file
// to list and draws its prompt; the picker's frame is held, so a test can read
// which picker the View flyout's R hotkey opened.
void showViewRecordingPicker(void);

// Show the main menu's View -> Game Stats screen, dismiss it, and quit the
// menu.  The stats screen is a transient full-screen display, so its frame is
// held for screenTextContains() and screenTextLineContaining() after the menu
// has closed.
void showGameStatsScreen(void);

// Show the main menu's View -> Game Stats screen with the run history forced
// to describe one completed run (`recentData` true) or no runs at all
// (`recentData` false), then put the player's run history back exactly as it
// was.  The stats frame is held for screenTextContains() and
// screenTextLineContaining().
void showGameStatsScreenWithRecentData(boolean recentData);

// Show the main menu's View -> High Scores screen, dismiss it, and quit the
// title screen that follows.  Because the menu returns to the title screen
// when the scores close, the frame captured at the final input request is
// that title screen; highScoresWereDisplayed() reports whether the scores
// were actually opened.
void showHighScoresFromView(void);

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

// The pointer states a menu button can be painted in.
enum menuButtonPointerState {
    MENU_BUTTON_POINTER_AWAY = 0,   // the pointer is not over the button
    MENU_BUTTON_POINTER_OVER = 1,   // the pointer rests over the button
    MENU_BUTTON_POINTER_HELD = 2    // the mouse button is held down on it
};

// Paint one menu button through the game's own menu painter with the pointer
// in the given state, and report the brightness of the middle of its
// background (0 is black, 300 is white) without touching the display.  The
// held-down state is the one the engine reaches when the pointer is over the
// button and the mouse button is down on it.
int menuButtonBackgroundBrightness(enum menuButtonPointerState pointer);
// The same for a label character at the middle of the button.
int menuButtonLabelBrightness(enum menuButtonPointerState pointer);

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

// Fill `values` with 0 through count - 1 and run the engine's own list shuffle
// over it, so a test can sample the ordering the game produces without
// touching the shuffle routine or the random-number source directly.
void shuffleTestSequence(short *values, short count);

// Draw one number from the game's own range roll with the given clumping
// factor: `clumpFactor` separate uniform rolls over the inclusive bounds,
// summed.  A factor above one clusters the results toward the middle of the
// range instead of spreading them evenly.
int drawClumpedRange(int lowerBound, int upperBound, int clumpFactor);

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

// Begin recording the current game, so a no-prompt recording save has a file
// to write.  One turn is recorded so the file is not empty.  Fails when the
// game is already in playback.
void beginTestRecording(void);

// Save the current recording without a prompt, exactly as the game's
// automatic saves do (server-mode death and victory, and the no-prompt
// close), and report whether a recording file was created.  Any file created
// is removed again, and any in-progress recording left behind is discarded,
// so a test leaves no recording files behind.
boolean saveRecordingWithoutPromptWritesFile(void);

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

// Whether the game is currently replaying a recording.
boolean isInPlaybackMode(void);
// Whether playback is currently paused.
boolean playbackIsPaused(void);
// Whether playback has found that the recording no longer matches the game.
boolean playbackIsOutOfSync(void);

// Set the deepest depth the recording being replayed explored.  A request to
// descend past it is refused with the "Already reached deepest depth explored"
// alert instead of seeking.
void setPlaybackDeepestDepthExplored(int depth);

// Declare the turn the recording being replayed ends on, so a go-to-turn
// request past it can be observed being capped there.  The recording must
// still hold enough recorded turns to replay up to that turn.
void setRecordingFinalTurn(int turn);

// Save the current game as a recording, then play it back through the main
// menu's View Recording command with the test's queued input, and quit the
// playback.  This takes the same path a player takes when choosing a
// recording from the title screen, so the state the game is left in is the
// state after a real playback ends.  Once it returns, the caller can check
// isInPlaybackMode(), playbackIsOutOfSync(), and the message log.
void viewCurrentRecordingAndQuitPlayback(void);

// Run as a server: the victory path skips the save-recording prompt and the
// high-score screen, so the game never asks for input.
void enableServerMode(void);

// Put the given amount of gold in the player's purse.
void givePlayerGold(int amount);

// The score a victory recorded right now would carry: gold plus the value of
// every carried item.
int expectedVictoryScore(void);

// Whether a high-score list has been drawn to the screen since the current
// test began.  The engine records this when it opens the high-score display,
// which is otherwise immediately covered by the prompts that follow a death.
boolean highScoresWereDisplayed(void);

// Engine-facing: remember that printHighScores() opened the score display.
void recordHighScoresDisplayed(void);

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
// The armor counterpart of givePlayerPlainWeapon(): a freshly generated plain
// (non-runic, uncursed) suit in the player's pack at an exact enchantment.
item *givePlayerPlainArmor(short kind, int enchant);
// Clear an item's identification (and any rune identity) so a test can observe
// behavior that only happens before the player learns what the item is.
void makeItemUnidentified(item *theItem);

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

// Whether the tile belongs to the first room the level generator carved on the
// level now in play.  The generator chooses and carves that room before any
// other room is attached, so this isolates it from the rest of the level.
// False when the level now in play was not generated during this run: a level
// restored from storage has no recorded first room.
boolean firstRoomCovers(pos loc);

// Framework internal: remember the first room the generator carved on the
// level now being built.  Called by the generator after carving that room and
// before attaching any other room.
void recordFirstRoomForTesting(short **grid);

// Framework internal: remember the tile an open-area vault's interior is
// growing from.  Called by the machine builder after it chooses the origin.
void recordOpenAreaVaultOriginForTesting(short x, short y);

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

// Build a vault on the current level whose doorway is guarded by a locked-door
// vestibule; that vestibule generates a door key and hands it to a child vault.
// Returns true when the vault (and its child) was built.  With
// childVaultsDisabled true no child vault can be built, so all of the vault's
// ten attempts fail and it must abort without altering the level.
boolean buildGuardedDoorVault(boolean childVaultsDisabled);

// A room-like vault's required room-size range: the doorway it grows from
// must have a blocked-off region of at least roomVaultRoomSizeMin() and at
// most roomVaultRoomSizeMax() tiles.
int roomVaultRoomSizeMin(void);
int roomVaultRoomSizeMax(void);

// Replace the current level with a synthetic one holding a single dead-end
// chamber of exactly roomCells floor tiles reached through one doorway, then
// try to build the room-like vault.  The chamber's floor is the region that
// would be cut off if its doorway were blocked.  Returns true when the vault
// was built.  Afterwards chamberFloorContains() and chamberDoorwayAt()
// identify the chamber's floor and its doorway.
boolean buildRoomVaultAtChamber(int roomCells);
int chamberFloorCellCount(void);
boolean chamberFloorContains(pos loc);
boolean chamberDoorwayAt(pos loc);

// Try to build the room-like vault on the current level, letting the engine
// choose a doorway.  Returns true when it was built; when no doorway on the
// level is eligible it returns false and leaves the level untouched.
boolean buildRoomVaultOnCurrentLevel(void);

// A fingerprint of every tile's terrain across all layers, so a test can
// compare the level before and after a build attempt.
unsigned long levelTerrainFingerprint(void);

// Build a room-like vault whose interior is rebuilt from scratch, using the
// same synthetic dead-end chamber as buildRoomVaultAtChamber().  The vault's
// room-size range is reported by redesignedVaultRoomSizeMin() and
// redesignedVaultRoomSizeMax(); roomCells should lie inside it.  Returns true
// when the vault was built.  While it builds, the engine records the
// route-length threshold its loop-adding pass used for the rebuilt interior;
// read that with vaultInteriorLoopThreshold().
boolean buildRedesignedRoomVaultAtChamber(int roomCells);
int redesignedVaultRoomSizeMin(void);
int redesignedVaultRoomSizeMax(void);
// Engine-facing: forget the loop-threshold record kept for vault rebuilds.
void resetVaultInteriorLoopRecord(void);
// The route-length threshold the engine's loop-adding pass used the last time
// it rebuilt a vault interior, or -1 when no rebuild has been recorded since
// the most recent buildRedesignedRoomVaultAtChamber() call.
int vaultInteriorLoopThreshold(void);
// Build a fixture vault interior and run the engine's pass that reconnects a
// vault's orphaned rooms: a `width` x `height` rectangle of solid interior rock
// whose north-west tile is already open floor and whose south-east tile is the
// door of a room the rebuilt layout left sealed off.  Everything outside the
// rectangle is off limits to the pass.  Tiles the pass tunnels through become
// floor and the rest of the rectangle stays granite, so a test can read the
// route straight off the level.  *openTile and *orphanTile receive the two
// corners.  Returns false when the rectangle does not fit on the level.
boolean buildOrphanedVaultInterior(int width, int height, pos *openTile, pos *orphanTile);

// Engine-facing: run the pass that reconnects a vault's orphaned rooms over a
// fixture interior (see buildOrphanedVaultInterior()).  `interior` marks the
// vault's tiles, `grid` holds the rebuilt layout (a positive value marks a tile
// that is already open, 0 marks interior rock), and `orphan` is the sealed
// room's door tile.  Tiles the pass tunnels through are set to a positive
// value in `grid`.
void reconnectOrphanedRoomsForTesting(char interior[DCOLS][DROWS], short **grid, pos orphan);

// Whether the engine's loop-adding pass cuts a doorway through a wall tile
// whose two flanking floor tiles are connected only by a route of `detour`
// tiles, when the pass uses `threshold` as its minimum route length.  The
// fixture is a U-shaped corridor; `detour` must be even and at least 4.
boolean loopPassCutsWallWithDetour(int detour, int threshold);

// Build an open-area vault through the engine's own location chooser, using
// the engine's actual depth-appropriate target-size range, and return true
// when it was built.  `openAreaVaultOrigin()` then reports the random floor
// tile the vault grew from.
boolean buildOpenAreaVault(void);
// The same, but growing from `origin` and aiming for exactly `targetSize`
// tiles.  Passing a fixed origin isolates the way the interior grows and the
// conditions that make it abort.  After a successful build the interior is
// identified by cellIsVaultInterior().
boolean buildOpenAreaVaultAt(pos origin, int targetSize);
// The tile the most recent open-area vault grew from.
pos openAreaVaultOrigin(void);
// The path distance from that origin to `loc`, or -1 when no route exists.
int openAreaVaultPathDistance(pos loc);
// The smallest and largest target size an open-area vault may choose.
int openAreaVaultTargetMin(void);
int openAreaVaultTargetMax(void);

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
// The red, green and blue components of the light on a tile.
void lightColorAt(pos loc, int *red, int *green, int *blue);

// Pathfinding distances between two tiles.  pathingDistanceBetween routes with
// no traveller (so undiscovered tiles are passable); playerPathingDistance
// routes as the player (so undiscovered tiles are impassable).
int pathingDistanceBetween(pos from, pos to, unsigned long blockingTerrainFlags);
int playerPathingDistance(pos from, pos to);
// The cost of the cheapest four-way route from `from` to `to` when entering
// every passable tile costs 1 except `expensiveSquare`, which costs
// `expensiveCost`.  A tile of the route's own entry cost is added each step,
// and 30000 means no route exists.
int pathingDistanceWithEntryCost(pos from, pos to, pos expensiveSquare, int expensiveCost);

// Timed lighting.
int minersLightRadius(void);
void setLightMultiplier(int multiplier);
void setPlayerInWater(boolean inWater);
void recomputeLighting(void);
// Recompute the player's field of view and redraw the map, as at the start of
// a turn, so tests can rely on visibility after moving terrain or creatures.
void recomputeVision(void);

// Play the engine's ordinary light-flash animation centred on `loc` and return
// the brightest total light that tile reached while the flash was drawn.  The
// flash is a generic one, so the tile is brighter during the animation than
// before it; once the animation ends its light is exactly what it was before.
int flashLightAtAndReportPeak(pos loc);
// Framework internal: note that the flash animation has just painted the
// light at (x, y), so the peak can be measured before the lighting is
// restored.  Called from animateFlares().
void recordTestFlareLight(short x, short y);

// The colour the player is currently drawn with: "invisible", "darkness",
// "shadow", "light", "torch orange" while burning, or "unknown".  The named
// colours are distinct.
const char *playerDrawColorName(void);
// Whether the player's drawn colour re-shades itself on every screen refresh
// (its colour "dances").
boolean playerDrawColorFlickers(void);
// The colour of the light the player carries, named for its role: "fire"
// while burning, "miners light" normally, or "other".
const char *playerLightColorName(void);
// Whether the carried light's colour re-shades itself on every screen refresh.
boolean playerLightColorFlickers(void);
// The red, green and blue components of the light the player carries.
void playerLightColorComponents(int *red, int *green, int *blue);
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
// The dungeon's deepest floor, below which there is no staircase to descend.
int dungeonDeepestLevel(void);
int totalGoldGenerated(void);
// Overwrite the running total of gold the dungeon has generated.  A level's
// gold-pile count is adjusted by 2 when this total is behind or ahead of the
// schedule for the current depth; pinning the total keeps that adjustment
// fixed so a test can observe the count the rule itself produces.
void setTotalGoldGenerated(int total);
int playerTurnCount(void);
// How many gold piles currently lie on the floor of the level.  Gold is
// generated separately from ordinary items, so countFloorItems() does not
// count it.
int countGoldPiles(void);
// The player's scent counter, which rises by 3 each turn (10 while the
// player is invisible) and is periodically reset to keep it in range.
int playerScentTurnNumber(void);

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

// How many items remain in the shared pool monsters are generated from.  Each
// monster generated holding an item takes that item out of the pool, so the
// count only falls.
int monsterItemHopperCount(void);

// Run the game's ordinary item and gold population for the current level,
// measuring placement from `upstairs` exactly as level generation does.  The
// generated items and gold accumulate on the floor.
void populateLevelItemsFrom(pos upstairs);

// Run one generation of the cellular automata that shapes dungeon features
// over a fixed test grid whose centre floor tile has all eight neighbours
// live, and report whether that centre tile is still floor.  `survivalRuleApplies`
// selects whether the survival rule for eight live neighbours keeps it.
boolean floorTileSurvivesGeneration(boolean survivalRuleApplies);

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
