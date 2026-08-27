# Building the Djinni DEB

`debian/` in this directory is a standard Debian packaging tree. Debian's
tooling expects `debian/` to live at the root of the source tree it's
packaging, so it isn't kept there directly -- it lives here instead, and
you copy or symlink it into place when you actually want to build.

This produces a single binary package, `libdjinni-dev`, containing
Djinni's headers, its pkg-config file, and its license/docs.

## Building from an official release

```sh
version=2.5.2

# Get the upstream release tarball and lay it out the way dpkg-source
# expects: an "orig" tarball, plus an extracted source tree with this
# packaging copied in as debian/.
curl -LO https://github.com/rjhansen/djinni/archive/refs/tags/v${version}.tar.gz
mv v${version}.tar.gz djinni_${version}.orig.tar.gz
tar xzf djinni_${version}.orig.tar.gz
mv djinni-${version} djinni-build && cd djinni-build
cp -a ../deb/debian .

# Build. -us -uc skips signing, for a local test build; drop them (and add
# -k<keyid>) when building something you intend to actually distribute.
dpkg-buildpackage -us -uc
```

Your `.deb`, `.dsc`, and `.changes` files land in the parent directory.

## Building from a local checkout, before a release exists

```sh
cd /path/to/djinni
version=2.5.2
git archive --prefix=djinni-${version}/ HEAD | gzip > /tmp/djinni_${version}.orig.tar.gz

mkdir /tmp/djinni-build && cd /tmp/djinni-build
mv /tmp/djinni_${version}.orig.tar.gz .
tar xzf djinni_${version}.orig.tar.gz
cp -a /path/to/djinni/deb/debian djinni-${version}/debian
cd djinni-${version}
dpkg-buildpackage -us -uc
```

## Notes

* Like the RPM spec in `../rpm/`, this is deliberately not architecture-
  independent (`Architecture: any`, not `all`) -- see the comment above
  `Package: libdjinni-dev` in `debian/control` for why.
* `dh --buildsystem=meson` drives the actual build; debhelper's Meson
  integration runs `meson setup`/`ninja`/`meson test`/`meson install`
  under the hood, so `dh_auto_test` exercises Djinni's own test suite as
  part of the package build.
* Bump the version in `debian/changelog` (with `dch -v` if you have
  `devscripts` installed) whenever you cut a new release.
* This was validated locally with `dpkg-source -b`, which confirmed
  `control`/`changelog`/`copyright`/`rules`/the source format all parse
  and assemble correctly into a real `.dsc` + source tarball. It was
  **not** validated with an actual `dpkg-buildpackage`/`debhelper` binary
  build, since debhelper itself isn't available in the environment this
  was written in -- if you have a real Debian/Ubuntu system or container,
  that's the way to confirm the rest.
