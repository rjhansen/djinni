Name:           djinni
Version:        2.5.2
Release:        1%{?dist}
Summary:        Header-only C++23 library implementing Ohlmann-Thomas compressed annealing

License:        ISC
URL:            https://github.com/rjhansen/djinni
# NOTE: adjust this to match the actual tag/archive layout of the release
# you are packaging -- GitHub's generated archive directory name depends on
# how the tag itself is named. This assumes a tag of "v%%{version}" whose
# archive extracts to a top-level "djinni-%%{version}" directory, which is
# GitHub's usual behavior for numeric version tags.
Source0:        https://github.com/rjhansen/djinni/archive/refs/tags/v%{version}/djinni-%{version}.tar.gz

# Djinni is header-only, so the package content itself (headers, license,
# docs) doesn't vary by architecture. It is deliberately NOT marked
# BuildArch: noarch, though: its pkgconfig file installs into the
# architecture-dependent %%{_libdir} (e.g. /usr/lib64 on most 64-bit
# platforms), and noarch packages that ship files under %%{_libdir} are a
# well-known source of multilib conflicts. Building one binary RPM per
# architecture avoids that entirely, at the modest cost of a few redundant
# builds of identical content.

BuildRequires:  meson >= 1.0.0
BuildRequires:  ninja-build
# Djinni uses C++23 concepts and other C++23 features; this needs a
# reasonably modern compiler. GCC 14 is what upstream develops and tests
# against (see the Compatibility Matrix in README.md) -- adjust if your
# target distribution's toolchain differs (e.g. clang, or an SCL/toolset
# package on older Enterprise Linux releases).
BuildRequires:  gcc-c++ >= 14
BuildRequires:  pkgconfig

%description
Djinni is a header-only C++23 library implementing Ohlmann-Thomas
compressed annealing, a simulated-annealing variant originally developed
for the Traveling Salesman Problem with Time Windows. It ships as a small
set of headers with no runtime dependencies beyond the C++23 standard
library, plus a pkg-config file so downstream projects can locate it with
`pkg-config --cflags djinni`.

There is nothing to link against: everything Djinni provides is templates
and inline functions in the installed headers.

%prep
%autosetup -n djinni-%{version}

%build
%meson
%meson_build

%check
%meson_test

%install
%meson_install

%files
%license LICENSE
%doc README.md NEWS AUTHORS
%{_includedir}/djinni.h
%{_includedir}/djinni/
%{_libdir}/pkgconfig/djinni.pc

%changelog
* Thu Aug 27 2026 Robert J. Hansen <rjh@sixdemonbag.org> - 2.5.2-1
- Initial RPM packaging.
