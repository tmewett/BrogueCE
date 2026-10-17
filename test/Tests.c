#include "TestFramework.h"

// Master registry of test suites.  Each rules file has its own suite source
// file (Tests_<Subsystem>.c) exporting a NULL-terminated array of test cases.
// Add new suites to this list.

extern const testCase tests_examples[];
extern const testCase tests_movement[];
extern const testCase tests_combat[];
extern const testCase tests_status_effects[];
extern const testCase tests_monsters[];
extern const testCase tests_items[];
extern const testCase tests_progression[];
extern const testCase tests_interaction[];
extern const testCase tests_dungeon[];
extern const testCase tests_bestiary[];
extern const testCase tests_death[];
extern const testCase tests_runic[];

const testCase *const testSuites[] = {
    tests_examples,
    tests_movement,
    tests_combat,
    tests_status_effects,
    tests_monsters,
    tests_items,
    tests_progression,
    tests_interaction,
    tests_dungeon,
    tests_bestiary,
    tests_death,
    tests_runic,
    NULL
};
