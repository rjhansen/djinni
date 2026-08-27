# Building the Djinni RPM

`djinni.spec` builds a single binary RPM containing Djinni's headers, its
pkg-config file, and its license/docs. It expects `rpmbuild`'s standard
`~/rpmbuild` tree (`rpmdev-setuptree` from the `rpmdevtools` package will
create one for you if you don't already have it).

## Building from an official release

```sh
spectool -g -R djinni.spec       # downloads Source0 into ~/rpmbuild/SOURCES
rpmbuild -ba djinni.spec
```

`spectool` comes from `rpmdevtools`. Your finished RPM (and SRPM) will be
under `~/rpmbuild/RPMS/` and `~/rpmbuild/SRPMS/`.

## Building from a local checkout, before a release exists

`Source0` points at a GitHub release tarball that won't exist until a
matching tag (`v2.5.2` as the spec currently stands) is actually pushed. To
test the spec against your working tree instead, generate an equivalent
tarball yourself and drop it in `~/rpmbuild/SOURCES/` under the name the
spec expects:

```sh
cd /path/to/djinni
git archive --prefix=djinni-2.5.2/ -o ~/rpmbuild/SOURCES/djinni-2.5.2.tar.gz HEAD
rpmbuild -ba rpm/djinni.spec
```

## Notes

* The spec is deliberately not `BuildArch: noarch` -- see the comment at
  the top of `djinni.spec` for why (the pkg-config file installs into the
  architecture-dependent library directory).
* `%check` runs Djinni's own Meson test suite (`meson test`), which needs a
  working C++23 compiler in the build environment -- the same requirement
  the library itself has.
* Bump `Version`, the `Source0` tag, and add a `%changelog` entry together
  whenever you cut a new release.
