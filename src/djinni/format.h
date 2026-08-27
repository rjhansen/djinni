/* Copyright (c) 2004 - 2026, Robert J. Hansen <rjh@sixdemonbag.org>
 * and Tristan D. Thiede (address currently unknown).
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. */

// This header is deliberately NOT included by djinni.h. <format> -- and
// especially <print>/<println>, which is what actually motivates having
// these formatters -- landed later, and less uniformly, across compilers
// and standard libraries than the rest of C++23. Forcing it on everyone
// who just wants Annealer/TravelingSalesmanSolution would risk breaking
// otherwise-supported toolchains for a feature most callers won't use.
// #include "djinni/format.h" explicitly if you want std::format,
// std::print, or std::println to work with Djinni's types.

#pragma once

#include "annealers.h"
#include "routes.h"
#include <format>
#include <sstream>

namespace std {
//! Makes any Annealer printable via std::format/std::print/std::println.
/*! This does not reimplement Annealer::dump(); it just funnels through the
   existing operator<<, so the JSON it produces is identical either way.
   Only the empty format spec ("{}") is supported -- there's no meaningful
   way to apply width/alignment/etc. to a multi-line JSON blob. */
template <edu::uiowa::tippie::djinni::PenaltyFunction PenaltyFunc,
          edu::uiowa::tippie::djinni::AnnealingSolution SolutionType>
struct formatter<
    edu::uiowa::tippie::djinni::Annealer<PenaltyFunc, SolutionType>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(
      const edu::uiowa::tippie::djinni::Annealer<PenaltyFunc, SolutionType> &a,
      format_context &ctx) const {
    ostringstream oss;
    oss << a;
    return format_to(ctx.out(), "{}", oss.str());
  }
};

//! Makes any TravelingSalesmanSolution printable via
//! std::format/std::print/std::println, by reusing its existing operator<<.
template <class WorldType>
struct formatter<
    edu::uiowa::tippie::djinni::TravelingSalesmanSolution<WorldType>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(
      const edu::uiowa::tippie::djinni::TravelingSalesmanSolution<WorldType> &s,
      format_context &ctx) const {
    ostringstream oss;
    oss << s;
    return format_to(ctx.out(), "{}", oss.str());
  }
};
} // namespace std
