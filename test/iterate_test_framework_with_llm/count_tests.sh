#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)/test"
grep -ohrE 'BR[0-9]{5}' Tests_*.c | sort -u | wc -l
grep -rhoE '\{ "BR[^"]+"' Tests_*.c | sed -E 's/\{ "//; s/"$//' | sort -u | wc -l
