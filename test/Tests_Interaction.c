// Tests derived from rules/07-player-interaction.md.
//
// The player-interaction rules are overwhelmingly about the screen: cursor
// control, the sidebar, the message archive, menus, file pickers, recordings,
// and help screens.  What can be checked headlessly is the behaviour of the
// two-button confirmation box, the way confirmations are skipped on autopilot,
// and the text of the dungeon-seed report.
//
// Omitted are all of the above screen-driven rules plus saving, loading,
// playback, filename handling, and the main menu.

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

const testCase tests_interaction[] = {
    { "BR56719_confirmation_accepts_y_Y_return",   yes_keys_setup,      NULL, confirmation_accepts_y_Y_return },
    { "BR19813_confirmation_rejects_n_N_space",    no_keys_setup,       NULL, confirmation_rejects_n_N_space },
    { "BR30441_escaping_a_confirmation_is_no",     escape_setup,        NULL, escaping_a_confirmation_is_no },
    { "BR67301_autopilot_skips_confirmation",      autopilot_setup,     NULL, autopilot_skips_confirmation },
    { "BR53953_seed_report_shows_seed_and_turn",   seed_report_setup,   NULL, seed_report_shows_seed_and_turn },
    { "BR78309_hidden_seed_is_reported_hidden",    hidden_seed_setup,   NULL, hidden_seed_is_reported_hidden },

    { NULL, NULL, NULL, NULL }
};
