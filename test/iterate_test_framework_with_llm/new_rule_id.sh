#!/usr/bin/env zsh

# Generate a new unique rule ID

set -euo pipefail
cd "$(git rev-parse --show-toplevel)/rules"

for i in $(seq 1 8); do
    id="BR$((RANDOM % 90000 + 10000))"
    if ! grep -rho "$id" .
    then 
        echo "$id"
        exit
    fi
done
