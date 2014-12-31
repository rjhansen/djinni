#!/bin/sh

aclocal -I m4
autoconf
autoheader
# note: linux systems should probably use libtoolize
glibtoolize
automake --copy --foreign --add-missing
