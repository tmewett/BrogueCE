#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(git rev-parse --show-toplevel)/rules"

# Rule IDs are written on a line of their own.  Matching only those lines keeps
# prose mentions out of the sample: rules/README.md documents the format with
# the example "BR39771", which is not a rule and has no test.
grep -hE '^BR[0-9]{5}$' "$SCRIPT_DIR"/* | sort -u | shuf -n 3

