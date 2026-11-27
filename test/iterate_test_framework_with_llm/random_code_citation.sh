#!/usr/bin/env zsh

# Selects a random file from src/brogue/* and a random block of source-code lines,
# then prints a GitHub citation link like:
#   [Architect.c:2892-2907](https://github.com/...)

set -euo pipefail

# ── Configuration ──────────────────────────────────────────────
REPO_ROOT="$(git rev-parse --show-toplevel)"
SRC_DIR="$REPO_ROOT/src/brogue"
BLOCK_SIZE=16            # number of consecutive code lines to cite
REPO_URL="https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue"
# ───────────────────────────────────────────────────────────────

lines=0
filename=""
cd "$SRC_DIR"

function pick_random_file() {
    # 1. Pick a random .c file
    filename=$(find *.c -maxdepth 1 -type f -print0 | shuf -z -n 1 | tr '\0' '\n')

    # echo "$filename"

    # 2. Extract only non-empty, non-comment-only lines with their line numbers.
    #    Skips blank lines, // comments, /* ... */ block comments, and lines inside block comments.
    code_lines=$(awk '
        /\/\*/ { in_block = 1 }
        in_block {
            if (/\*\//) { in_block = 0 }
            next
        }
        /^[[:space:]]*$/ { next }
        /^[[:space:]]*\/\// { next }
        { print NR }
    ' "$filename")

    # 3. Convert newline-separated string to a zsh array
    lines=(${(f)code_lines})
    # Remove any empty entries that can sneak in
    lines=(${lines:#})
}

for i in {1..10}; do

    pick_random_file
    # echo "$lines"
    # echo "${#lines[@]}"

    if (( ${#lines[@]} > BLOCK_SIZE )); then
        # 4. Pick a random starting index, clamped so we don't run past the end
        start_idx=0
        max_start=$(( ${#lines[@]} - BLOCK_SIZE ))
        if (( max_start < 0 )); then
            BLOCK_SIZE=${#lines[@]}
            max_start=0
        else
            start_idx=$(( RANDOM % (max_start + 1) ))
        fi

        # 5. Build the range
        first_line=${lines[$start_idx]}
        last_line=${lines[$(( start_idx + BLOCK_SIZE - 1 ))]}

        # 6. Output the citation
        echo "[${filename}:${first_line}-${last_line}](${REPO_URL}/${filename}#L${first_line}-L${last_line})"
        exit 0
    fi
done
