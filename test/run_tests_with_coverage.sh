#!/bin/zsh
set -e
cd "$(git rev-parse --show-toplevel)"
mkdir -p prof
make GRAPHICS=NO TERMINAL=NO MAC_APP=NO COVERAGE=YES >/dev/null
fail=0; total=0
for n in $(grep -rhoE '\{ "BR[^"]+"' test/Tests_*.c | sed -E 's/\{ "//; s/"$//'); do
  total=$((total+1))
  echo "$n"
  out=$(LLVM_PROFILE_FILE="prof/code-%p.profraw" ./bin/brogue --test "$n" | tail -3)
  if ! echo "$out" | grep -q PASS; then
    echo "FAIL $n"
    fail=$((fail+1))
  fi
done
echo "total=$total fails=$fail"
exit $fail