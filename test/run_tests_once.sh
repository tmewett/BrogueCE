#!/bin/zsh
set -e
cd "$(git rev-parse --show-toplevel)"
# Force a rebuild: object files do not depend on the build flags, so a
# previous non-test build would otherwise leave test hooks uncompiled.
make -B GRAPHICS=NO TERMINAL=NO MAC_APP=NO TEST_FRAMEWORK=YES >/dev/null
fail=0; total=0
SECONDS=0

function run_test() {
  out=$(./bin/brogue --test "$1" | tail -3)
  if ! echo "$out" | grep -q PASS; then
    echo "FAIL: $1"
    echo "$out"
    fail=$((fail+1))
    return 0
  fi
}

for n in $(grep -rhoE '\{ "BR[^"]+"' test/Tests_*.c | sed -E 's/\{ "//; s/"$//'); do
  total=$((total+1))
  echo "$n"
  run_test "$n"
done
echo "total=$total fails=$fail"
echo "Time: $SECONDS sec"
exit $fail