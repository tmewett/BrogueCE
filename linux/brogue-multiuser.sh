#!/bin/sh

# This is a sample script for running Brogue when installed to a system-wide
# directory, while keeping saves, recordings and highscores in user-specific
# locations. Modify as necessary and place as /usr/local/bin/brogue or
# elsewhere.
#
# ATTENTION: To use this, you must have built Brogue CE from source having set
# the DATADIR build option.

set -e

broguedir="/opt/brogue"  # where you set DATADIR to (copy the contents of 'bin' to here)
userdir="${XDG_DATA_HOME:-$HOME/.local/share}/Brogue"  # where you want user files to go

mkdir -p "$userdir"
cd "$userdir"
exec "$broguedir"/brogue "$@"
