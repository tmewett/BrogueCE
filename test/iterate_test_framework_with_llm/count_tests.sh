#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)/test"
UNIQ=$(grep -ohrE 'BR[0-9]{5}' Tests_*.c | sort -u | wc -l)
TOTL=$(grep -rhoE '\{ "BR[^"]+"' Tests_*.c | sed -E 's/\{ "//; s/"$//' | sort -u | wc -l)
echo "Tests: $UNIQ unique, $TOTL total"
