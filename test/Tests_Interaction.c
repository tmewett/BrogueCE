// Tests derived from rules/07-player-interaction.md.
//
// The player-interaction rules are overwhelmingly about the screen: cursor
// control, the sidebar, the message archive, menus, file pickers, recordings,
// and help screens.  What can be checked headlessly is the behaviour of the
// two-button confirmation box, how prompts are cancelled by Escape, space, or
// a click outside them, the way confirmations are skipped on autopilot,
// the text of the dungeon-seed report, the color a message is shown in,
// whether Return turns on the
// keyboard cursor, and — by queueing keystrokes through the engine's main
// input loop — what confirming the cursor on the player's own location does.
//
// Omitted are all of the above screen-driven rules (aside from the keyboard
// cursor's on/off state, the sidebar's poison labels and health bars, the
// section list of the discovered-items screen, the no-matching-files alert of
// the file picker, and confirming the cursor on the player's own location)
// plus saving, loading, the rest of playback (advancing with control aside,
// and its bottom-row buttons), the rest of filename handling, and the main
// menu apart from its Change Variant dialog.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// ============================================================
// Confirmation box
// ============================================================

static boolean yesLower, yesUpper, yesReturn;

static void yes_keys_setup(void) {
    queueKeystroke('y', false, false);
    yesLower = askConfirmation();
    queueKeystroke('Y', false, false);
    yesUpper = askConfirmation();
    queueKeystroke(RETURN_KEY, false, false);
    yesReturn = askConfirmation();
}

static boolean confirmation_accepts_y_Y_return(void) {
    TEST_ASSERT(yesLower, "y should answer a confirmation Yes");
    TEST_ASSERT(yesUpper, "Y should answer a confirmation Yes");
    TEST_ASSERT(yesReturn, "Return should answer a confirmation Yes");
    return !testFailed;
}

static boolean noLower, noUpper, noSpace;

static void no_keys_setup(void) {
    queueKeystroke('n', false, false);
    noLower = askConfirmation();
    queueKeystroke('N', false, false);
    noUpper = askConfirmation();
    queueKeystroke(ACKNOWLEDGE_KEY, false, false);
    noSpace = askConfirmation();
}

static boolean confirmation_rejects_n_N_space(void) {
    TEST_ASSERT(!noLower, "n should answer a confirmation No");
    TEST_ASSERT(!noUpper, "N should answer a confirmation No");
    TEST_ASSERT(!noSpace, "space should answer a confirmation No");
    return !testFailed;
}

static boolean escapeWasNo;

static void escape_setup(void) {
    queueKeystroke(ESCAPE_KEY, false, false);
    escapeWasNo = askConfirmation();
}

static boolean escaping_a_confirmation_is_no(void) {
    TEST_ASSERT(!escapeWasNo, "Escape should answer a confirmation No");
    return !testFailed;
}

// ============================================================
// BR25868 — Escape, space, or clicking outside the menu cancels the prompt
// ============================================================

static boolean cancelControlAccepted;
static boolean cancelSpaceWorked, cancelEscapeWorked, cancelClickWorked;

static void cancel_prompt_setup(void) {
    // Control: the prompt is working and still accepts a Yes key.
    queueKeystroke('y', false, false);
    cancelControlAccepted = askConfirmation();

    queueKeystroke(ACKNOWLEDGE_KEY, false, false); // space
    cancelSpaceWorked = !askConfirmation();

    queueKeystroke(ESCAPE_KEY, false, false);
    cancelEscapeWorked = !askConfirmation();

    // A click far outside the text box, at the top-left corner of the screen,
    // followed by a Yes key.  If the outside click cancels, the prompt ends at
    // the click and the Yes key is never offered to it; if it does not, the
    // queued Yes key answers the still-open prompt instead.
    queueMouseClick(posAt(1, 1));
    queueKeystroke('y', false, false);
    cancelClickWorked = !askConfirmation();
}

static boolean prompt_cancels_on_escape_space_or_outside_click(void) {
    TEST_ASSERT(cancelControlAccepted,
                "the prompt should still accept a Yes key (control)");
    TEST_ASSERT(cancelSpaceWorked,
                "space should cancel the prompt");
    TEST_ASSERT(cancelEscapeWorked,
                "Escape should cancel the prompt");
    TEST_ASSERT(cancelClickWorked,
                "clicking outside the menu should cancel the prompt");
    return !testFailed;
}

// ============================================================
// Autopilot
// ============================================================

static boolean autopilotWasYes;

static void autopilot_setup(void) {
    enableAutopilot();
    autopilotWasYes = askConfirmation(); // no key queued
}

static boolean autopilot_skips_confirmation(void) {
    TEST_ASSERT(autopilotWasYes,
                "a confirmation on autopilot should be treated as Yes");
    return !testFailed;
}

// ============================================================
// Dungeon seed report
// ============================================================

static void seed_report_setup(void) {
    showDungeonSeed();
}

static boolean seed_report_shows_seed_and_turn(void) {
    char expected[64];
    snprintf(expected, sizeof(expected), "#%llu", dungeonSeed());
    TEST_ASSERT(messageWasShown("Dungeon seed"),
                "the seed report should mention the dungeon seed");
    TEST_ASSERT(messageWasShown(expected),
                "the seed report should contain the seed number");
    TEST_ASSERT(messageWasShown("turn #"),
                "the seed report should contain the turn number");
    return !testFailed;
}

static void hidden_seed_setup(void) {
    setDungeonSeedHidden(true);
    showDungeonSeed();
}

static boolean hidden_seed_is_reported_hidden(void) {
    TEST_ASSERT(messageWasShown("HIDDEN"),
                "the seed report should say HIDDEN when the seed is hidden");
    return !testFailed;
}

// ============================================================
// BR50315 — pressing Return/Enter turns on the keyboard cursor
// ============================================================

static boolean cursorWasActive;

static void keyboard_cursor_setup(void) {
    clearArena();
    cursorWasActive = isKeyboardCursorActive();
}

static boolean keyboard_cursor_step(void) {
    injectKeystroke(RETURN_KEY, false, false);
    return false;
}

static boolean return_turns_on_keyboard_cursor(void) {
    TEST_ASSERT(!cursorWasActive,
                "the keyboard cursor should start out turned off");
    TEST_ASSERT(isKeyboardCursorActive(),
                "pressing Return should turn on the keyboard cursor");
    return !testFailed;
}

// ============================================================
// BR33110 — the Feats screen lists every feat with its achievement status
// and closes on any key or click
// ============================================================

static void feats_screen_setup(void) {
    clearArena();
    // Earn a feat deterministically so that an "achieved" status exists for the
    // feats screen to list (killing a dragon earns the dragonslayer feat).
    pos dragonLoc = posOffset(player.loc, 1, 0);
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    creature *dragon = placeHostileMonster(MK_DRAGON, dragonLoc);
    setMonsterHealth(dragon, 1);
    applyStatusToMonster(dragon, STATUS_PARALYZED, 20);
    playerAttacksMonster(dragon);
}

static boolean feats_screen_opens_and_closes(void) {
    // Opening the feats screen should not end the game; with no input queued,
    // test mode answers the "press any key to continue" prompt with escape, so
    // the screen closes and play resumes.
    injectKeystroke(FEATS_KEY, false, false);
    return false;
}

static boolean feats_achievement_status_tracked(void) {
    TEST_ASSERT(playerHasFeat(FEAT_DRAGONSLAYER),
                "an earned feat should be recorded as achieved");
    TEST_ASSERT(!rogue.gameHasEnded,
                "opening and closing the feats screen should not end the game");
    return !testFailed;
}

// ============================================================
// BR32227 — the player's own sidebar entry shows Strength, Armor, and Gold
// ============================================================

static void sidebar_gold_setup(void) {
    clearArena();
    // Pick up a pile of gold so the sidebar has an amount to show.
    placeItemOnFloor(GOLD, 0, posOffset(getPlayerPos(), 1, 0));
    moveRight();
}

static boolean sidebar_shows_strength_armor_and_gold(void) {
    TEST_ASSERT(sidebarContains("Str:"),
                "the sidebar should show your strength");
    TEST_ASSERT(sidebarContains("Armor:"),
                "the sidebar should show your armor value");
    TEST_ASSERT(sidebarContains("Gold:"),
                "the sidebar should show your gold");
    return !testFailed;
}

static void sidebar_penniless_setup(void) {
    clearArena();
}

static boolean sidebar_omits_gold_when_penniless(void) {
    TEST_ASSERT(sidebarContains("Str:"),
                "the sidebar should still show your strength");
    TEST_ASSERT(sidebarContains("Armor:"),
                "the sidebar should still show your armor value");
    TEST_ASSERT(!sidebarContains("Gold:"),
                "with no gold to show, the sidebar should leave out the gold line");
    return !testFailed;
}

// ============================================================
// BR13184 — repeats of an identical message collapse into one
// counted entry, which reads "(many)" at the repeat maximum
// ============================================================

static boolean repeat_same_message_five_step(void) {
    for (int i = 0; i < 5; i++) {
        emitArchivedMessage("the dust settles");
    }
    return false;
}

static boolean repeats_rendered_as_counted_entry(void) {
    const char *line = renderedMessageLine();
    TEST_ASSERT(strstr(line, "dust settles (x5)") != NULL,
                "five identical messages should render as one entry counted (x5)");
    TEST_ASSERT(strstr(line, "(many)") == NULL,
                "below the repeat maximum the entry should show a number, not (many)");
    return !testFailed;
}

static boolean repeat_same_message_to_max_step(void) {
    for (int i = 0; i < MAX_MESSAGE_REPEATS; i++) {
        emitArchivedMessage("the dust settles");
    }
    return false;
}

static boolean max_repeats_rendered_as_many(void) {
    const char *line = renderedMessageLine();
    TEST_ASSERT(strstr(line, "dust settles (many)") != NULL,
                "at the repeat maximum the entry should read (many) instead of a number");
    TEST_ASSERT(strstr(line, "(x") == NULL,
                "at the repeat maximum no count number should be shown");
    return !testFailed;
}

// ============================================================
// BR23017 — poison that would kill the target before it wears off is
// labeled "Fatal Poison", and stacked poison is shown with its strength,
// e.g. "Poisoned (x3)"
// ============================================================

static void fatal_poison_setup(void) {
    clearArena();
    // Five hit points: ten turns of one-strength poison outlasts them.
    setPlayerHealth(5);
    poisonPlayer(10, 1);
}

static boolean lethal_poison_labeled_fatal(void) {
    TEST_ASSERT(isPlayerPoisoned(),
                "precondition: the player is poisoned");
    TEST_ASSERT(playerHealth() <= playerStatusTurns(STATUS_POISONED) * playerPoisonConcentration(),
                "precondition: the poison would kill before it wears off");
    TEST_ASSERT(sidebarContains("Fatal Poison"),
                "poison that would kill before it wears off should be labeled Fatal Poison");
    TEST_ASSERT(!sidebarContains("Poisoned (x"),
                "a single-strength poison should not be shown with a stack count");
    return !testFailed;
}

static void surviving_poison_setup(void) {
    clearArena();
    // Sixty hit points: ten turns of one-strength poison wears off first.
    setPlayerHealth(60);
    poisonPlayer(10, 1);
}

static boolean nonlethal_poison_labeled_poisoned(void) {
    TEST_ASSERT(isPlayerPoisoned(),
                "precondition: the player is poisoned");
    TEST_ASSERT(playerHealth() > playerStatusTurns(STATUS_POISONED) * playerPoisonConcentration(),
                "precondition: the poison wears off before it would kill");
    TEST_ASSERT(sidebarContains("Poisoned"),
                "surviving poison should be labeled Poisoned");
    TEST_ASSERT(!sidebarContains("Fatal Poison"),
                "poison that would not kill should not be labeled Fatal Poison");
    return !testFailed;
}

static void stacked_poison_setup(void) {
    clearArena();
    // Sixty hit points: ten turns of three-strength poison amount to 30,
    // so it wears off before it would kill.
    setPlayerHealth(60);
    poisonPlayer(10, 3);
}

static boolean stacked_poison_shows_strength(void) {
    TEST_ASSERT_EQ(playerPoisonConcentration(), 3,
                   "precondition: the poison strength is");
    TEST_ASSERT(playerHealth() > playerStatusTurns(STATUS_POISONED) * playerPoisonConcentration(),
                "precondition: the poison wears off before it would kill");
    TEST_ASSERT(sidebarContains("Poisoned (x3)"),
                "stacked poison should be shown with its strength, e.g. Poisoned (x3)");
    TEST_ASSERT(!sidebarContains("Fatal Poison"),
                "poison that would not kill should not be labeled Fatal Poison");
    return !testFailed;
}

// ============================================================
// BR21675 — confirming the cursor while it sits on the player's own
// location does nothing
// ============================================================

static pos confirmOwnLocationStart;
static unsigned long confirmOwnLocationTurn;

static void confirm_own_location_setup(void) {
    clearArena();
    confirmOwnLocationStart = getPlayerPos();
    confirmOwnLocationTurn = turnNumber();
    // Return turns the keyboard cursor on, which drops it onto the player's
    // own location; a second Return confirms that very location.  Both
    // keystrokes are queued so the engine's main input loop, where cursor
    // confirmation lives, processes them like a real game would.
    queueKeystroke(RETURN_KEY, false, false);
    queueKeystroke(RETURN_KEY, false, false);
}

static boolean confirm_own_location_step(void) {
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean confirming_own_location_does_nothing(void) {
    TEST_ASSERT(playerIsAt(confirmOwnLocationStart),
                "confirming the cursor on the player's own location should leave the player put");
    TEST_ASSERT_EQ((int)turnNumber(), (int)confirmOwnLocationTurn,
                   "confirming the cursor on the player's own location should spend no turn");
    return !testFailed;
}

// ============================================================
// BR72215 — the discovered-items screen lists scrolls, rings, potions,
// staffs, and wands
// ============================================================

static boolean discoveriesListedBeforeOpening;

static void discoveries_screen_setup(void) {
    clearArena();
}

static boolean open_discoveries_screen_step(void) {
    // Whatever the screen lists must come from opening it, so record what was
    // on screen beforehand.
    discoveriesListedBeforeOpening = screenTextContains("-- SCROLLS --");
    injectKeystroke(DISCOVERIES_KEY, false, false);
    return false;
}

static boolean discoveries_screen_lists_all_five_categories(void) {
    TEST_ASSERT(!discoveriesListedBeforeOpening,
                "no discoveries section should be listed before the screen is opened");
    TEST_ASSERT(screenTextContains("-- SCROLLS --"),
                "the discovered-items screen should list scrolls");
    TEST_ASSERT(screenTextContains("-- RINGS --"),
                "the discovered-items screen should list rings");
    TEST_ASSERT(screenTextContains("-- POTIONS --"),
                "the discovered-items screen should list potions");
    TEST_ASSERT(screenTextContains("-- STAFFS --"),
                "the discovered-items screen should list staffs");
    TEST_ASSERT(screenTextContains("-- WANDS --"),
                "the discovered-items screen should list wands");
    return !testFailed;
}

// ============================================================
// BR26817 — if a file picker finds no matching files, it shows
// "No applicable files found."
// ============================================================

static boolean noFileChosen;

static void no_matching_files_setup(void) {
    queueKeystroke(RETURN_KEY, false, false); // dismiss the alert box
    noFileChosen = chooseFileWithSuffix(".brogue-no-such-suffix");
}

static boolean no_applicable_files_alert_shows(void) {
    TEST_ASSERT(!noFileChosen,
                "a file picker that finds no matching file should not select one");
    TEST_ASSERT(screenTextContains("No applicable files"),
                "a file picker with no matching files should show \"No applicable files found.\"");
    TEST_ASSERT(screenTextContains("found."),
                "the alert should carry the whole \"No applicable files found.\" text");
    return !testFailed;
}

// ============================================================
// BR93927 — holding control while advancing moves 20 turns
// ============================================================

static int turnsBeforeControlAdvance;
static int turnsAfterControlAdvance;

#define CONTROL_ADVANCE_ARMED_TURNS 30

static void control_advance_setup(void) {
    clearArena(); // open floor for the steps the seek will replay
    // The replayed steps must not land on a staircase.
    placeDownStairs(posAt(1, 1));
    placeUpStairs(posAt(levelWidth() - 2, levelHeight() - 2));
    // The helper walks its own copy of the steps to record the recording's
    // out-of-sync checks, so the turn count is read afterwards.
    startPlaybackWithRecordedChecks(CONTROL_ADVANCE_ARMED_TURNS);
    turnsBeforeControlAdvance = playerTurnCount();
}

static boolean control_advance_step(void) {
    queueKeystroke(RIGHT_ARROW, false, true); // hold control while advancing
    // The same steps startPlaybackWithRecordedChecks() recorded: thirty
    // single-turn steps, of which a correct advance consumes twenty; the
    // leftovers are not playback controls, so they cannot advance further.
    for (int i = 0; i < CONTROL_ADVANCE_ARMED_TURNS; i++) {
        queueKeystroke((i % 2) ? DOWNLEFT_KEY : UPRIGHT_KEY, false, false);
    }
    runQueuedInputThroughMainLoop();
    turnsAfterControlAdvance = playerTurnCount();
    return false;
}

static boolean control_advancing_moves_twenty_turns(void) {
    TEST_ASSERT_MSG(turnsAfterControlAdvance - turnsBeforeControlAdvance == 20,
                    "holding control while advancing should move exactly 20 turns");
    return !testFailed;
}

// ============================================================
// BR97169 — the Change Variant dialog describes and lets the player pick
// Rapid Brogue, (classic) Brogue, or Bullet Brogue; canceling leaves the
// choice unchanged
// ============================================================

static const char *variantAfterRapid, *variantAfterBullet, *variantAfterClassic;

static void pick_variants_setup(void) {
    showVariantDialog('r');
    variantAfterRapid = gameVariantName();
    showVariantDialog('l');
    variantAfterBullet = gameVariantName();
    showVariantDialog('b');
    variantAfterClassic = gameVariantName();
}

static boolean variant_dialog_picks_each_variant(void) {
    TEST_ASSERT(messageWasShown("Classic Brogue"),
                "the variant dialog should describe classic Brogue");
    TEST_ASSERT(messageWasShown("Rapid Brogue"),
                "the variant dialog should describe Rapid Brogue");
    TEST_ASSERT(messageWasShown("Bullet Brogue"),
                "the variant dialog should describe Bullet Brogue");
    TEST_ASSERT(strcmp(variantAfterRapid, "Rapid Brogue") == 0,
                "picking Rapid Brogue should select Rapid Brogue");
    TEST_ASSERT(strcmp(variantAfterBullet, "Bullet Brogue") == 0,
                "picking Bullet Brogue should select Bullet Brogue");
    TEST_ASSERT(strcmp(variantAfterClassic, "Brogue") == 0,
                "picking Brogue should select the classic Brogue variant");
    return !testFailed;
}

static void cancel_keeps_choice_setup(void) {
    showVariantDialog('r'); // start from a choice that differs from the default
    clearMessages();        // so the next dialog's text proves it was shown again
}

static boolean cancel_variant_dialog_step(void) {
    showVariantDialog(ESCAPE_KEY);
    return false;
}

static boolean canceling_variant_dialog_keeps_choice(void) {
    TEST_ASSERT(messageWasShown("Rapid Brogue"),
                "the variant dialog should be shown again before it is canceled");
    TEST_ASSERT(strcmp(gameVariantName(), "Rapid Brogue") == 0,
                "canceling the variant dialog should leave the choice unchanged");
    return !testFailed;
}

// ============================================================
// BR42937 — a monster's sidebar entry shows a health bar if its
// maximum health is above 1 and it is not invulnerable
//
// The bar's label carries a percentage exactly when the creature's health
// differs from the "previous health" the last player-turn end recorded, so
// each case below arranges which entries gain one: the player's own bar is
// the control, either left plain (case one) or given a percentage (cases two
// and three), and the monster's is the subject under test.  A plain bar
// reads "Health" followed by padding spaces, a percentage bar reads
// "Health (" with the parenthesised percentage - so a single row tells them
// apart: "Health  " (two spaces after the word) marks a plain bar, and
// "Health (" marks a bar carrying a percentage.
// ============================================================

static creature *br42937_ordinary;

static void br42937_bar_setup(void) {
    clearArena();
    br42937_ordinary = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    // Paralyzed: the rat neither moves nor strikes back, so the player stays
    // at full health and only the rat's bar below gains a percentage.
    applyStatusToMonster(br42937_ordinary, STATUS_PARALYZED, 100);
    restOneTurn(); // the turn end records every creature's health as full
    setMonsterHealth(br42937_ordinary, monsterMaxHealth(br42937_ordinary) / 2);
}

static boolean br42937_ordinary_monster_has_health_bar(void) {
    TEST_ASSERT(monsterMaxHealth(br42937_ordinary) > 1,
                "precondition: the rat's maximum health is above 1");
    TEST_ASSERT(sidebarContains("Rat"),
                "precondition: the rat's entry is listed in the sidebar");
    TEST_ASSERT(sidebarContains("Health ("),
                "an ordinary monster's sidebar entry should show its health bar, "
                "here with a percentage because it is hurt");
    return !testFailed;
}

static creature *br42937_oneHp;

static void br42937_one_hp_setup(void) {
    clearArena();
    br42937_oneHp = placeHostileMonster(MK_SPECTRAL_IMAGE, posOffset(getPlayerPos(), 1, 0));
    applyStatusToMonster(br42937_oneHp, STATUS_PARALYZED, 100);
    restOneTurn(); // the turn end records both sides at full health
    // Only the player's bar gains a percentage, so any plain "Health" row
    // left in the sidebar must come from the monster's own bar.
    setPlayerHealth(playerMaxHealth() / 2);
}

static boolean br42937_one_hp_monster_has_no_health_bar(void) {
    TEST_ASSERT_EQ(monsterMaxHealth(br42937_oneHp), 1,
                   "precondition: the spectral sword's maximum health is exactly 1");
    TEST_ASSERT(sidebarContains("Spectral sword"),
                "precondition: the spectral sword's entry is listed in the sidebar");
    TEST_ASSERT(sidebarContains("Health ("),
                "precondition: the sidebar is drawn - the player's own hurt bar shows a percentage");
    TEST_ASSERT(!sidebarContains("Health  "),
                "a monster with 1 maximum health should get no health bar of its own");
    return !testFailed;
}

static creature *br42937_warden;

static void br42937_invulnerable_setup(void) {
    clearArena();
    br42937_warden = placeHostileMonster(MK_WARDEN_OF_YENDOR, posOffset(getPlayerPos(), 1, 0));
    applyStatusToMonster(br42937_warden, STATUS_PARALYZED, 100);
    restOneTurn();
    setPlayerHealth(playerMaxHealth() / 2);
}

static boolean br42937_invulnerable_monster_has_no_health_bar(void) {
    TEST_ASSERT(monsterHasTrait(br42937_warden, MONST_INVULNERABLE),
                "precondition: the warden is invulnerable");
    TEST_ASSERT(monsterMaxHealth(br42937_warden) > 1,
                "precondition: the warden's maximum health is above 1");
    TEST_ASSERT(sidebarContains("Warden of Yendor"),
                "precondition: the warden's entry is listed in the sidebar");
    TEST_ASSERT(sidebarContains("Health ("),
                "precondition: the sidebar is drawn - the player's own hurt bar shows a percentage");
    TEST_ASSERT(!sidebarContains("Health  "),
                "an invulnerable monster's sidebar entry should get no health bar");
    return !testFailed;
}

// ============================================================
// BR65948 — during playback the bottom row of the screen shows
// buttons for Unpause, Omniscience, Next Turn, and Menu
// ============================================================

static void playback_buttons_setup(void) {
    clearArena();
    enterPlaybackMode();
}

static boolean playback_buttons_step(void) {
    // The main input loop builds the bottom row before it draws the frame and
    // asks the test for input; with nothing queued, that request ends the
    // pump and leaves the frame captured for the assertions.
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean playback_shows_playback_buttons(void) {
    TEST_ASSERT(screenTextContains("Unpause"),
                "during playback the bottom row should show an Unpause button");
    TEST_ASSERT(screenTextContains("Omniscience"),
                "during playback the bottom row should show an Omniscience button");
    TEST_ASSERT(screenTextContains("Next Turn"),
                "during playback the bottom row should show a Next Turn button");
    TEST_ASSERT(screenTextContains("Menu"),
                "during playback the bottom row should show a Menu button");
    TEST_ASSERT(!screenTextContains("Explore"),
                "the normal-play bottom row should not appear during playback");
    return !testFailed;
}

// ============================================================
// BR14809 — Each character of a button's label is drawn at the
// button's own position, but any character that would land
// outside the display window is skipped, so a label running
// past the edge of the screen is simply cut off there instead
// of being drawn off-screen.
//
// The label is 120 characters wide on the 100-column screen
// and centred like any dialog, so its first ten characters
// would land left of the window and its last ten past the
// right edge: only the middle hundred characters should reach
// the screen, and neither end may appear anywhere on it.
// ============================================================

static const char br14809_label[] =
    "LEFTCUT>>"
    "xxxxxxxxxx" "xxxxxxxxxx" "xxxxxxxxxx" "xxxxxxxxxx" "xxxxxxxxxx"
    "xxxxxxxxxx" "xxxxxxxxxx" "xxxxxxxxxx" "xxxxxxxxxx" "xxxxxxxxxx"
    "x"
    "<<RIGHTCUT";

static boolean br14809_shown;

static void br14809_setup(void) {
    br14809_shown = showOversizedButton(br14809_label);
}

static boolean br14809_label_clipped_at_screen_edge(void) {
    TEST_ASSERT(br14809_shown,
                "Return should choose the button (control: it was drawn and took input)");
    TEST_ASSERT(screenTextContains("xxxxxxxxxx"),
                "the part of the label that fits on screen should be drawn");
    TEST_ASSERT(!screenTextContains("LEFTCUT>>"),
                "the label's left end lands outside the window and should be skipped, not drawn");
    TEST_ASSERT(!screenTextContains("<<RIGHTCUT"),
                "the label's right end runs past the screen edge and should be cut off there");
    return !testFailed;
}

// ============================================================
// BR77643 — The title screen offers New Game (N), Play (P),
// View (V), and Quit (Q).
//
// showTitleScreen() runs the real main menu: it draws the title
// screen, waits for one key, and quits on it, so the frame the
// engine captured at that wait is the title screen with all of
// its entries, readable afterwards with screenTextContains().
// ============================================================

static void br77643_setup(void) {
    showTitleScreen();
}

static boolean br77643_verify(void) {
    TEST_ASSERT(screenTextContains("New Game"),
                "the title screen should offer New Game (N)");
    TEST_ASSERT(screenTextContains("Play"),
                "the title screen should offer Play (P)");
    TEST_ASSERT(screenTextContains("View"),
                "the title screen should offer View (V)");
    TEST_ASSERT(screenTextContains("Quit"),
                "the title screen should offer Quit (Q)");
    return !testFailed;
}

// ============================================================
// BR31995 — Message color reflects importance: harm to the
// player or an ally is shown in the "bad" color, harm to an
// enemy is shown in the "good" color, and neutral events are
// white.
//
// Three staged blows cover the three verdicts, and the
// dungeon-seed report stands in for a neutral event.  Every
// staged blow is guaranteed: a paralyzed defender (the enemy,
// the ally) or a stuck defender (you) is always hit, and both
// victims are raised to outlast their blows, so no death message
// can substitute for the hit message.  Paralysis lifts the moment
// a creature is struck, so a victim may strike back during the
// resting turn; but the staged combat messages are buffered
// first and flushed in order, so the color read for each
// substring comes from the staged blow.  Each combat message
// names one of the creatures, so the three are told apart by
// substring.  Combat messages are buffered until the turn ends,
// so the colors are read after a resting turn has flushed them
// into the log.
// ============================================================

static const char *br31995_enemyColor;
static const char *br31995_playerColor;
static const char *br31995_allyColor;
static const char *br31995_neutralColor;

static void br31995_setup(void) {
    clearMonsters();
    setArenaRadius(4, FLOOR);
    pos here = getPlayerPos();
    creature *enemy  = placeHostileMonster(MK_RAT,    posOffset(here, 1, 0));
    creature *jackal = placeHostileMonster(MK_JACKAL, posOffset(here, -1, 0));
    creature *kobold = placeHostileMonster(MK_KOBOLD, posOffset(here, 0, 2));
    creature *ally   = spawnMonster(MK_MONKEY,        posOffset(here, 0, 1));
    makeMonsterAlly(ally);
    // Reveal the ally's tile, as placeHostileMonster() does for the others:
    // a creature you cannot see is named "something" in combat messages.
    revealCell(posOffset(here, 0, 1));

    applyStatusToMonster(enemy,  STATUS_PARALYZED, 10000);
    applyStatusToMonster(jackal, STATUS_PARALYZED, 10000);
    applyStatusToMonster(kobold, STATUS_PARALYZED, 10000);
    applyStatusToMonster(ally,   STATUS_PARALYZED, 10000);
    applyStatusToPlayer(STATUS_STUCK, 10000);
    // Both victims outlast their blows, so no death message can substitute
    // for the hit message its color is read from.
    setMonsterHealth(enemy, 100);
    setMonsterHealth(ally, 100);

    playerAttacksMonster(enemy);   // harm to an enemy
    monsterAttacksPlayer(jackal);  // harm to the player
    attackMonster(kobold, ally);   // harm to an ally
    showDungeonSeed();             // a neutral event
    restOneTurn();                 // the turn's end flushes the combat messages

    br31995_enemyColor   = messageColorOf("the rat");
    br31995_playerColor  = messageColorOf("jackal");
    br31995_allyColor    = messageColorOf("monkey");
    br31995_neutralColor = messageColorOf("Dungeon seed");
}

static boolean message_color_reflects_importance(void) {
    TEST_ASSERT(br31995_enemyColor != NULL
                && strcmp(br31995_enemyColor, "good") == 0,
                "harm to an enemy should be shown in the good color");
    TEST_ASSERT(br31995_playerColor != NULL
                && strcmp(br31995_playerColor, "bad") == 0,
                "harm to the player should be shown in the bad color");
    TEST_ASSERT(br31995_allyColor != NULL
                && strcmp(br31995_allyColor, "bad") == 0,
                "harm to an ally should be shown in the bad color");
    TEST_ASSERT(br31995_neutralColor != NULL
                && strcmp(br31995_neutralColor, "white") == 0,
                "neutral events should be shown in white");
    return !testFailed;
}

// ============================================================
// BR95103 — canceling the cursor (with space or Escape) hides
// it and clears the highlighted travel path.
//
// Both tests pump the engine's real input loop: Return turns the
// keyboard cursor on, two eastward moves carry it (and, being
// two cells from you, draw the travel path), and then the
// canceling keystroke runs the loop's cancel branch.
//
// The hide test ends its queue with a space: hideCursor() is the
// only thing that can turn the cursor off again before the pump
// returns, so if canceling did not hide it, the assertion fails.
//
// The path test ends its queue after the two moves: the frame
// captured at the input request that finds the queue dry has the
// cursor and the lit path still drawn (the capture happens before
// the loop's end-of-queue Escape cancels the session), so that
// frame can be remembered; one more pump then redraws the map
// without them, and the path cells must read as changed against
// the remembered frame — proving the path was shown and was then
// cleared — while a cell the path never crossed must not.
// ============================================================

static void br95103_arena_setup(void) {
    setArenaRadius(4, FLOOR);
    // The travel path only draws over discovered terrain, so reveal the
    // two path cells (and a cell north of you that no path here crosses).
    revealCell(posOffset(getPlayerPos(), 1, 0));
    revealCell(posOffset(getPlayerPos(), 2, 0));
    revealCell(posOffset(getPlayerPos(), 0, -2));
}

static void br95103_hide_setup(void) {
    br95103_arena_setup();
    queueKeystroke(RETURN_KEY, false, false);      // turn the keyboard cursor on
    queueKeystroke(RIGHT_KEY, false, false);       // move it one cell east
    queueKeystroke(RIGHT_KEY, false, false);       // and a second cell east
    queueKeystroke(ACKNOWLEDGE_KEY, false, false); // space cancels the cursor
}

static boolean br95103_hide_step(void) {
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean br95103_canceling_hides_cursor(void) {
    TEST_ASSERT(!isKeyboardCursorActive(),
                "canceling the cursor with space should hide it");
    return !testFailed;
}

static void br95103_path_setup(void) {
    br95103_arena_setup();
    queueKeystroke(RETURN_KEY, false, false); // turn the keyboard cursor on
    queueKeystroke(RIGHT_KEY, false, false);  // move it one cell east
    queueKeystroke(RIGHT_KEY, false, false);  // and a second cell east
    // When the queue runs dry the input hook synthesizes an Escape event,
    // which cancels the cursor.  The frame captured at that empty fetch —
    // the pump's last — still shows the cursor and its lit travel path.
}

static boolean br95103_path_step(void) {
    // The pump's last captured frame is taken at the empty fetch, just
    // before the hook synthesizes the Escape that cancels the cursor, so it
    // still shows the lit travel path.  Remember that frame, then take one
    // more frame of the game after the cancel has been processed.
    runQueuedInputThroughMainLoop();
    rememberScreenColors();
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean br95103_canceling_clears_travel_path(void) {
    TEST_ASSERT(screenCellColorsChanged(getPlayerPos()),
                "canceling the cursor should clear the highlighted travel path");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), 0, -2)),
                "a cell the travel path never crossed should keep its colour");
    return !testFailed;
}

const testCase tests_interaction[] = {
    { "BR32227_sidebar_shows_strength_armor_and_gold", sidebar_gold_setup,     NULL, sidebar_shows_strength_armor_and_gold },
    { "BR32227_sidebar_omits_gold_when_penniless",    sidebar_penniless_setup, NULL, sidebar_omits_gold_when_penniless },

    { "BR42937_ordinary_monster_shows_health_bar",    br42937_bar_setup,        NULL, br42937_ordinary_monster_has_health_bar },
    { "BR42937_one_hp_monster_shows_no_health_bar",   br42937_one_hp_setup,     NULL, br42937_one_hp_monster_has_no_health_bar },
    { "BR42937_invulnerable_monster_shows_no_health_bar", br42937_invulnerable_setup, NULL, br42937_invulnerable_monster_has_no_health_bar },

    { "BR56719_confirmation_accepts_y_Y_return",   yes_keys_setup,      NULL, confirmation_accepts_y_Y_return },
    { "BR19813_confirmation_rejects_n_N_space",    no_keys_setup,       NULL, confirmation_rejects_n_N_space },
    { "BR30441_escaping_a_confirmation_is_no",     escape_setup,        NULL, escaping_a_confirmation_is_no },
    { "BR25868_prompt_cancels_on_escape_space_or_outside_click", cancel_prompt_setup, NULL, prompt_cancels_on_escape_space_or_outside_click },
    { "BR67301_autopilot_skips_confirmation",      autopilot_setup,     NULL, autopilot_skips_confirmation },
    { "BR53953_seed_report_shows_seed_and_turn",   seed_report_setup,   NULL, seed_report_shows_seed_and_turn },
    { "BR78309_hidden_seed_is_reported_hidden",    hidden_seed_setup,   NULL, hidden_seed_is_reported_hidden },
    { "BR50315_return_turns_on_keyboard_cursor",   keyboard_cursor_setup, keyboard_cursor_step, return_turns_on_keyboard_cursor },
    { "BR33110_feats_screen_lists_achievement_status_and_closes", feats_screen_setup, feats_screen_opens_and_closes, feats_achievement_status_tracked },

    { "BR13184_repeats_collapse_into_counted_entry", NULL, repeat_same_message_five_step, repeats_rendered_as_counted_entry },
    { "BR13184_max_repeats_rendered_as_many",        NULL, repeat_same_message_to_max_step, max_repeats_rendered_as_many },

    { "BR23017_lethal_poison_labeled_fatal",   fatal_poison_setup,       NULL, lethal_poison_labeled_fatal },
    { "BR23017_nonlethal_poison_labeled_poisoned", surviving_poison_setup, NULL, nonlethal_poison_labeled_poisoned },
    { "BR23017_stacked_poison_shows_strength", stacked_poison_setup,     NULL, stacked_poison_shows_strength },
    { "BR72215_discoveries_screen_lists_all_five_categories", discoveries_screen_setup, open_discoveries_screen_step, discoveries_screen_lists_all_five_categories },

    { "BR26817_file_picker_shows_no_applicable_files", no_matching_files_setup, NULL, no_applicable_files_alert_shows },
    { "BR93927_control_advancing_moves_20_turns",       control_advance_setup,   control_advance_step, control_advancing_moves_twenty_turns },

    { "BR65948_playback_bottom_row_buttons",            playback_buttons_setup,  playback_buttons_step, playback_shows_playback_buttons },

    { "BR21675_confirming_cursor_on_own_location_does_nothing", confirm_own_location_setup, confirm_own_location_step, confirming_own_location_does_nothing },

    { "BR14809_label_clipped_at_screen_edge",    br14809_setup,   NULL,                          br14809_label_clipped_at_screen_edge },

    { "BR97169_change_variant_dialog_picks_each_variant",    pick_variants_setup,      NULL,                          variant_dialog_picks_each_variant },
    { "BR97169_canceling_variant_dialog_keeps_choice",        cancel_keeps_choice_setup, cancel_variant_dialog_step,   canceling_variant_dialog_keeps_choice },

    { "BR77643_title_screen_offers_new_game_play_view_quit", br77643_setup, NULL, br77643_verify },

    { "BR31995_message_color_reflects_importance", br31995_setup, NULL, message_color_reflects_importance },

    { "BR95103_canceling_cursor_hides_it",         br95103_hide_setup,  br95103_hide_step,  br95103_canceling_hides_cursor },
    { "BR95103_canceling_cursor_clears_travel_path", br95103_path_setup, br95103_path_step,  br95103_canceling_clears_travel_path },

    { NULL, NULL, NULL, NULL }
};
