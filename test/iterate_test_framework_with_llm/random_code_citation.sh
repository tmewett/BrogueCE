#!/usr/bin/env zsh
# random_code_citation.sh
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

# 1. Pick a random .c / .h file
files=( "$SRC_DIR"/*.c "$SRC_DIR"/*.h )
file=${files[$(( RANDOM % ${#files[@]} ))]}
filename="${file:t}"

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
' "$file")

# 3. Convert newline-separated string to a zsh array
lines=(${(f)code_lines})
# Remove any empty entries that can sneak in
lines=(${lines:#})

if (( ${#lines[@]} == 0 )); then
    echo "(no source-code lines found in $filename)" >&2
    exit 1
fi

# 4. Pick a random starting index, clamped so we don't run past the end
max_start=$(( ${#lines[@]} - BLOCK_SIZE ))
if (( max_start < 0 )); then
    BLOCK_SIZE=${#lines[@]}
    max_start=0
fi
start_idx=$(( RANDOM % (max_start + 1) ))

# 5. Build the range
first_line=${lines[$start_idx]}
last_line=${lines[$(( start_idx + BLOCK_SIZE - 1 ))]}

# 6. Output the citation
echo "[${filename}:${first_line}-${last_line}](${REPO_URL}/${filename}#L${first_line}-L${last_line})"
