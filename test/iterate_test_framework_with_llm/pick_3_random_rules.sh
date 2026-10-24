#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(git rev-parse --show-toplevel)/rules"

grep -ohE 'BR[0-9]{5}' "$SCRIPT_DIR"/*.md | shuf -n 3

