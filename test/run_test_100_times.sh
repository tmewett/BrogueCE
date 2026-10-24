#!/bin/zsh
set -e
cd "$(git rev-parse --show-toplevel)"
make GRAPHICS=NO TERMINAL=NO MAC_APP=NO >/dev/null
fail=0; total=0

function run_100_times() {
  for i in {1..100}
  do
      total=$((total+1))
      out=$(./bin/brogue --test "$1" | tail -3)
      if ! echo "$out" | grep -q PASS; then
        echo "FAIL after $i tries: $1"
        fail=$((fail+1))
        return 0
      fi
  done
}

echo "$1"
run_100_times "$1"
echo "total=$total fails=$fail"
exit $fail