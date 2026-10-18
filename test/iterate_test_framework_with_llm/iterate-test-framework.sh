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

function writeToLog() {
    echo "$1\n" >> llm_log.txt
}

function validate_random_rules() {
    writeToLog "validate_random_rules"

    # Pick random rule IDs and validate they meet rule requirements
    RULES=$(pick_3_random_rules.sh)
    writeToLog $RULES

    PRMPT="OBJECTIVE
Verify that rule IDs $RULES meet the following requirements.

REQUIREMENTS
Rules for the game Brogue are collated into files in the directory "rules/*".
Rules MUST describe in-game behavior in one or two natural-language sentences.
Rules MUST NOT name data structures, functions, or modules from the source code, but rather use in-game terminology for their descriptions.
Rules MUST cite source code lines where that behavior is implemented in the game code.
Rules MUST describe a single, simple, observable behavior.
Rules MUST be assigned an ID beginning with \"BR\" followed by 5 random digits (such as "BR39812").
Rule IDs MUST be unique across all rule files.
"
    writeToLog $PRMPT

    # Execute the prompt
    OUT=$(opencode -p $PRMPT)
    writeToLog $OUT
}

while true; do

    # Pick random rule IDs and validate they meet rule requirements

    # Pick random rule IDs and validate they have tests

    # Pick random rule IDs and validate the tests match the rules and that the tests pass

    # Pick random source code citation and validate its behavior is described by a rule

    # Pick a random rule which has no test and implement it

    # Pick a random test which has no rule and write it

    # Run all tests and verify passing, else pick one test to fix

    # # Pick 3 random files safely
    # mapfile -d $'\0' -t files < <(find raw-inbox -maxdepth 1 -type f -print0 | shuf -z -n 3)
    # if [ ${#files[@]} -gt 0 ]; then
    #     echo "------"
    #     date
    #     echo "Adding new files to the wiki: ${files[*]}"
    #     echo "------"

    #     # Add the new file to the wiki
    #     sbx run opencode-bryan-wiki -- run "Implement the instructions in llm-wiki.md. Existing wiki is in ./wiki, original documents are in ./raw-sources. Add ${files[@]} to the wiki, expand and cross-link associated pages."

    #     # Move the new files to raw-sources
    #     mv "${files[@]}" raw-sources
    #     echo "------"
    #     echo "COMPLETED adding new files to the wiki: ${files[*]}"
    #     date
    #     echo "------"

    #     # Commit the gardening to the repo
    #     git add wiki
    #     git commit -m "Added files ${files[*]}"
    # else
    #     echo "------"
    #     echo "No new files in ./raw-inbox.  Pausing before gardening."
    #     echo "------"
    #     sleep 5m
    # fi

    # # Choose several files from the wiki (mindepth 2 to get to the actual pages, not overview/log/etc.)
    # FILES=$(find ./wiki -mindepth 2 -type f -print0 | shuf -z -n 3 | xargs -0 -I {} echo "{}")
    # echo "------"
    # date
    # echo "Gardening exising wiki files:"
    # echo "${FILES}"
    # echo "------"

    # # Call the LLM to garden the wiki (provide current date)
    # DATE=$(date)
    # sbx run opencode-bryan-wiki -- run "The current date is ${DATE}. Implement the instructions in llm-wiki.md. Compare the content of the existing wiki pages ${FILES}. Summarize or trim a page if it contains duplicate information. Add cross-links if the pages contain related information, remove dead links. If the pages disagree or are inconsistent with each other, add a note and link pointing this out. Review the referenced sources from ./raw-sources, update or clarify the pages if they have drifted from the referenced source document. Add new references when appropriate."

    # echo "------"
    # echo "COMPLETED gardening exising wiki files:"
    # echo "${FILES}"
    # date
    # echo "------"

    # # Commit the gardening to the repo
    # git add wiki
    # git commit -m "Gardening ${FILES}"

    # # Regenerate the HTML pages
    # python3 wiki2html-onepage.py
done
