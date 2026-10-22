#!/opt/homebrew/bin/bash

set -euo pipefail

#
# WHAT IS THIS?
#
# The Brogue test suite (rules/*, test/*) is generated and 
# maintained by AI tools with active human oversight and curation.
#
# This script is a sequence of prompts which can direct AI tools
# to iterate and improve on the test suite.
#
# It was designed to be used by opencode along with local AI models
# and run in a loop overnight.  Every day, the updates are reviewed
# by a human and either committed or rejected.
#
# After some time, the shape of the framework, rules, and tests
# should stabilize and allow shifting into a fine-tuning mode rather
# than the brute-force sequence of phases described here.
#
#
# ITERATION STEPS:
# - [GARDENING] Pick random existing rule IDs and verify they meet the requirements --> record issues in RulesNeedImprovement.txt
# - [GARDENING] Pick random existing rule IDs and verify they have tests associated with them --> record missing tests in RulesMissingTests.txt
# - [GARDENING] Pick random existing test IDs and verify the test has a rule, matches the rule, and the test passes --> record issues in TestsMissingRules.txt and TestsNeedImprovement.txt
# - [EXTENSION] Pick random code block and check that its behavior is specified by some rule(s) --> add new behaviors to NewRules.txt
# - [BACKFILL] Pick RulesMissingTests.txt --> write a test
# - [BACKFILL] Pick TestsMissingRules.txt --> write a rule
# - [STABILIZE] Run all tests and fix any failing tests
# - [COVERAGE] Run all tests with coverage, choose a poorly-covered region and write a rule that references it
# - [IMPROVEMENT] RulesNeedImprovement.txt --> refine the rule
# - [IMPROVEMENT] TestsNeedImprovement.txt --> refine the test framework to improve the tests
#
#
# LOCAL LLM TOOLING
#
# A local LLM can be run using llama.cpp, like this (MacOS, settings for an MacBook Pro M4 Max):
#    brew install llama.cpp
#    llama serve -hf unsloth/Qwen3.6-35B-A3B-GGUF:Q6_K --metrics --flash-attn on -ngl 9999 --ctx-size 0 --jinja -ub 4096 -b 4096 -t 10 --threads-batch 10 --chat-template-kwargs '{"preserve_thinking": true}' --port 8012 --alias qwen-local
#
# OpenCode supports local LLMs via a config file, like this:
# .config/opencode/opencode.json
# {
#   "$schema": "https://opencode.ai/config.json",
#   "provider": {
#     "llama-local": {
#       "name": "Llama.cpp",
#       "npm": "@ai-sdk/openai-compatible",
#       "options": {
#         "baseURL": "http://localhost:8012/v1"
#       },
#       "models": {
#         "unsloth/Qwen3.6-35B-A3B-GGUF:Q6_K": {
#           "name": "Qwen3.6-35B-A3B:Q6_K"
#         }
#       }
#     }
#   }
# }

REPO_ROOT=$(git rev-parse --show-toplevel)
cd "$REPO_ROOT/test/iterate_test_framework_with_llm"

function writeToLog() {
    echo "$1" >> llm_log.txt
}

function write_date() {
    # Mark time passing
    DATE=$(date)
    writeToLog "--------------------------------------------"
    writeToLog "$DATE"
    writeToLog "--------------------------------------------"
}

# --------------------------------------------
function validate_random_rules() {
    write_date
    writeToLog "validate_random_rules"

    # Pick random rule IDs and validate they meet rule requirements
    RULES=$(./pick_3_random_rules.sh)
    writeToLog "$RULES"

PRMPT="OBJECTIVE
Verify that rule IDs $RULES meet the following requirements.

REQUIREMENTS
Rules for the game Brogue are collated into files in the directory rules/*.
Rules MUST describe in-game behavior in one or two natural-language sentences.
Rules MUST NOT name data structures, functions, or modules from the source code, but rather use in-game terminology for their descriptions.
Rules MUST cite source code lines where that behavior is implemented in the game code.
Rules MUST describe a single, simple, observable behavior.
Rules MUST be assigned an ID beginning with BR followed by 5 random digits (such as BR39812). Use new_rule_id.sh to generate a new rule ID.
Rule IDs MUST be unique across all rule files.

OUTPUT
Rules not meeting requirements SHOULD be appended to the file test/RulesNeedImprovement.txt with a short note describing the issue.
"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"
}


# --------------------------------------------
function validate_random_rules_have_tests() {
    write_date
    writeToLog "validate_random_rules_have_tests"

    # Pick random rule IDs and validate they meet rule requirements
    RULES=$(./pick_3_random_rules.sh)
    writeToLog "$RULES"

PRMPT="OBJECTIVE
Verify that rule IDs $RULES meet the following requirements.

REQUIREMENTS
Rules MUST have one or more associated tests in test/*
Tests for a rule MUST include the rule ID in the test name.

OUTPUT
Rule IDs without tests SHOULD be appended to test/RulesMissingTests.txt only append the rule ID, no commentary.  Deduplicate the file to keep it clean.
"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"
}


# --------------------------------------------
function validate_random_tests() {
    write_date
    writeToLog "validate_random_tests"

    # Pick random rule IDs and validate they meet test requirements
    RULES=$(./pick_3_random_rules.sh)
    writeToLog "$RULES"

PRMPT="
OBJECTIVE
Verify that tests for rule IDs $RULES meet the following requirements.

REQUIREMENTS
Tests MUST be associated with a rule found in rules/*
If a rule has more than one test, they MUST include a positive and negative test.
Tests SHOULD NOT reference game engine components or data structures.  Use TestFramework functions which hide engine details instead.
Tests MUST be falsifiable.  If a test succeeds when its test assertion is negated, it is not a valid test.

OUTPUT
Tests without associated rules SHOULD be appended to test/TestsMissingRules.txt with a short note describing the issue.
Tests which directly examine engine components should be noted in test/TestsNeedImprovement.txt with a short note describing the issue.
"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"
}


# --------------------------------------------
function find_missing_rules() {
    write_date
    writeToLog "find_missing_rules"

    # Pick random source code citation
    CITE=$(./random_code_citation.sh)
    writeToLog "$CITE"

PRMPT="
OBJECTIVE
Verify that the in-game behaviors described by source code at $CITE is captured by one or more rules in rules/*

REQUIREMENTS
Each observable in-game behavior MUST have a rule describing it in rules/*
Rules MUST describe in-game behavior in one or two natural-language sentences.
Each rule MUST cite the source code where the in-game behavior is implemented.
Rules MUST be assigned an ID beginning with BR followed by 5 random digits (such as BR39812). Use new_rule_id.sh to generate a new rule ID.
Rule IDs MUST be unique across all rule files.  Use new_rule_id.sh to generate new rule IDs.

OUTPUT
Missing rules SHOULD be added to rules/NewRules.txt with a short note and precise citation for the missing behavior.
"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"
}


# --------------------------------------------
function write_missing_tests() {

    # Check if there are any tests missing rules
    FILE="$REPO_ROOT/test/RulesMissingTests.txt"
    line_count=$([ -f "$FILE" ] && wc -l < "$FILE" || echo 0)
    if [ "$line_count" -eq 0 ]; then
        return 0
    fi

    write_date
    writeToLog "write_missing_tests"

PRMPT="
OBJECTIVE
The tests for rules in test/RulesMissingTests.txt are missing.  Write tests which meets the requirements below and add it to test/*

REQUIREMENTS
Tests SHOULD NOT reference game engine components or data structures.  Use TestFramework functions which hide engine details instead.
Tests MUST be falsifiable.  If a test succeeds when its test assertion is negated, it is not a valid test.

Build Brogue for testing using: make GRAPHICS=NO TERMINAL=NO MAC_APP=NO
Run a specific test using: ./bin/brogue --test <<TEST_NAME>>
"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"
}


# --------------------------------------------
function write_missing_rules() {

    # Check if there are any tests missing rules
    FILE="$REPO_ROOT/test/TestsMissingRules.txt"
    line_count=$([ -f "$FILE" ] && wc -l < "$FILE" || echo 0)
    if [ "$line_count" -eq 0 ]; then
        return 0
    fi

    write_date
    writeToLog "write_missing_rules"

PRMPT="
OBJECTIVE
The rules for tests in test/TestsMissingRules.txt are missing.
Write rules which meet the requirements below and add it to rules/*
Make sure the test functions and test name are rewritten to include the correct rule ID.

REQUIREMENTS
Rules for the game Brogue are collated into files in the directory rules/*
Rules MUST describe in-game behavior in one or two natural-language sentences.
Rules MUST NOT name data structures, functions, or modules from the source code, but rather use in-game terminology for their descriptions.
Rules MUST cite source code lines where that behavior is implemented in the game code.
Rules MUST describe a single, simple, observable behavior.
Rules MUST be assigned an ID beginning with BR followed by 5 random digits (such as BR39812). Use new_rule_id.sh to generate a new rule ID.
Rule IDs MUST be unique across all rule files.
"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"
}


# --------------------------------------------
function code_coverage() {
    write_date
    writeToLog "code_coverage"

    # Run all tests, collect test coverage data
    $REPO_ROOT/test/run_tests.sh
    xcrun llvm-profdata merge -sparse $REPO_ROOT/prof/code-*.profraw -o coverage.profdata

    # Display low-coverage regions/lines
    xcrun llvm-cov show ../../bin/brogue -instr-profile=coverage.profdata -region-coverage-lt=1 -ignore-filename-regex='test|\.h' > low_coverage_regions.txt
    # xcrun llvm-cov show $REPO_ROOT/bin/brogue -instr-profile=coverage.profdata -line-coverage-lt=1 -ignore-filename-regex='.*[Tt]est.*'

    # Display coverage report
    # xcrun llvm-cov report $REPO_ROOT/bin/brogue -instr-profile=coverage.profdata

    awk -f convert_coverage.awk low_coverage_regions.txt > low_coverage_citations.txt

    # TODO: Select a region of low coverage and mine it for rules
    CITE=$(cat low_coverage_citations.txt | shuf -n 1)

PRMPT="
OBJECTIVE
The code at ($CITE) has low coverage, write a new rule in rules/NewRules.txt which will inspire a new test to improve the code coverage for this citation.

REQUIREMENTS
Each observable in-game behavior MUST have a rule describing it in rules/*
Rules MUST describe in-game behavior in one or two natural-language sentences.
Each rule MUST cite the source code where the in-game behavior is implemented.
Rules MUST be assigned an ID beginning with BR followed by 5 random digits (such as BR39812). Use new_rule_id.sh to generate a new rule ID.
Rule IDs MUST be unique across all rule files.  Use new_rule_id.sh to generate new rule IDs.
"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"

}


# --------------------------------------------
function fix_broken_tests() {
    write_date
    writeToLog "fix_broken_tests"

    PRMPT="Run all tests using test/run_tests.sh and fix any tests which are failing.  Only modify files in test/*"
    writeToLog "$PRMPT"

    # Execute the prompt
    OUT=$(opencode run --standalone "$PRMPT")
    writeToLog "$OUT"
}

function delete_epmemeral_files() {
    # rm -f $REPO_ROOT/test/RulesNeedImprovement.txt
    # rm -f $REPO_ROOT/test/TestsNeedImprovement.txt
    rm -f $REPO_ROOT/test/RulesMissingTests.txt
    rm -f $REPO_ROOT/test/TestsMissingRules.txt

    rm -f $REPO_ROOT/prof/code-*.profraw
    rm -f coverage.profdata
    rm -f low_coverage_regions.txt
    rm -f low_coverage_citations.txt
}


# Remove partial work from last invocation
delete_epmemeral_files

# --------------------------------------------
# Gardening the test cases...
for i in {1..100}; do
    # Pick random rule IDs and validate they meet rule requirements
    # validate_random_rules

    # Pick random rule IDs and validate they have tests
    # validate_random_rules_have_tests

    # Pick random rule IDs and validate the tests match the rules and that the tests pass
    # validate_random_tests

    # Pick random source code citation and validate its behavior is described by a rule
    # find_missing_rules

    # Pick a rule which has no test and implement it
    write_missing_tests

    # Pick a test which has no rule and write it
    write_missing_rules

    # Run all tests and verify passing, fix broken tests
    fix_broken_tests

    # Locate uncovered code and write a rule that attempts to capture it
    # code_coverage

    # TODO: RulesNeedImprovement, TestsNeedImprovement

    # TODO: Look for test framework components that can be simplified / abstracted

    # TODO: Look for tests which are redundant, or step functions that do the same thing but have different names, etc..

    # Clean up from this iteration
    delete_epmemeral_files
done

write_date
# Commit the updates to the repo
# git add rules
# git add test
# git commit -m "Test framework iteration ${date}"

