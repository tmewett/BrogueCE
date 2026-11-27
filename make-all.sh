# Test all build shapes

# TODO:
# - Set this up to fail if any of the builds fails
# - Output only errors so the failing builds are easy to find
# - Probably best to set all the vars to avoid config.mk defaults:
#   > make -B GRAPHICS=NO TERMINAL=NO WEBBROGUE=NO GRAPHICS=NO DEBUG=NO RELEASE=NO MAC_APP=NO

make -B GRAPHICS=NO

make -B TERMINAL=YES
make -B WEBBROGUE=YES
make -B GRAPHICS=YES
make -B DEBUG=YES
make -B RELEASE=YES
make -B MAC_APP=YES

# make -B JSBROGUE=YES
# make -B TABLET=YES
# make -B COVERAGE=YES
