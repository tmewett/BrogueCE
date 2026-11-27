#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)/test/iterate_test_framework_with_llm"
./all_rules.sh > junk_all_rules.txt
./all_tested_rules.sh > junk_tested_rules.txt
grep -Fxvf junk_tested_rules.txt junk_all_rules.txt | sort -u | shuf -n 3
rm junk_all_rules.txt
rm junk_tested_rules.txt
