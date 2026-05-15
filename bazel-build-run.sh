#!/usr/bin/env bash

bazelisk build //...

cp bazel-bin/brogue bin/

cd "$(dirname "$0")/bin" || exit

./brogue $*
