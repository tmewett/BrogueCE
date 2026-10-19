#!/bin/zsh
set -e
cd "$(git rev-parse --show-toplevel)"
make GRAPHICS=NO TERMINAL=NO MAC_APP=NO COVERAGE=YES >/dev/null
fail=0; total=0
for n in $(grep -rhoE '\{ "BR[^"]+"' test/Tests_*.c | sed -E 's/\{ "//; s/"$//'); do
  total=$((total+1))
  # out=$(./bin/brogue --test "$n" 2>/dev/null | tail -3)
  out=$(LLVM_PROFILE_FILE="code-%p.profraw" ./bin/brogue --test "$n" | tail -3)
  if ! echo "$out" | grep -q PASS; then
    echo "FAIL $n"
    fail=$((fail+1))
  fi
done
echo "total=$total fails=$fail"
exit $fail