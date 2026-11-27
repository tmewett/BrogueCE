#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)/test"
grep -rhoE '\{ "BR[^"]+"' Tests_*.c | sed -E 's/\{ "//; s/"$//' | sort -u
