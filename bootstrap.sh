#!/bin/sh

aclocal -I m4
autoconf
autoheader
automake --copy --foreign --add-missing

# note: linux systems should probably use libtoolize
glibtoolize

