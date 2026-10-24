#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)/rules"
grep -ohrE 'BR[0-9]{5}' . | sort -u | wc -l
