#!/bin/sh

set -e

ARGUMENTS="$*"

cd $(dirname "$0")/..
cat .gitmodules | \
while true; do
    read module || break
    read line; set -- $line
    path=$3
    read line; set -- $line
    url=$3
    read line; set -- $line
    branch=$3

    if [ -z "$ARGUMENTS" ]; then
        ARGUMENTS="--depth 1"
    fi

    git clone --filter=blob:none $url $path -b $branch --recursive $ARGUMENTS
done
