# Djinni

## The Bottom Line Up Front
You want this if you need a modern C++ annealing library that’s

* **Free.** Free to use, free to share, free to modify, free to
  learn from. It’s released under the ISC license.
* **Not a library at all.** It’s all done with header files: there’s
  literally nothing to build except a sample program meant to show
  you how to integrate the headers with your own code.
* **Works Freakin’ Everywhere.™** It’s regularly tested against
  Windows 11, MacOS Sequoia, and Linux. It’s also been recently run
  on FreeBSD, OpenBSD, and NetBSD. And if you’re willing to curse a
  little bit, it’ll even run on [Haiku](https://www.haiku-os.org).
  The parade of architectures is also something to behold: it’s 
  regularly built on x86_64 and Apple Silicon, but in the past ran on
  MIPS, PA-RISC, DEC Alpha, and Itanium.
* **Resource-friendly.** In the words of the Valgrind memory checker,
  `All heap blocks were freed -- no leaks are possible`. The example
  program provided with Djinni approximates a solution to a 100-city
  traveling salesman problem (with time windows!) in only 100k of 
  memory on x86_64.
* **Easy to interface.** I’ve even compiled this to WebAssembly and
  packaged it up into client-side web applications, doing annealing
  at nearly native speeds in somebody’s web browser. Python, Rust, and
  JavaScript are considered high-priority targets, and it isn’t hard
  to make them sing.
* **Built to professional standards.** The design and code are both
  clean and (reasonably) documented.
* **Not just based on published research, IS published research.**
  The theoretical basis was published in the 
  [operations research literature](https://myweb.uiowa.edu/bthoa/DownloadItems/TSPTWpaper4-05-05.pdf),
  and the version 2.0 series of the Djinni library was presented at
  CodeCon.
  - Ohlmann, J., J. Bean, and S. Henderson (2004).
  Convergence in probability of compressed annealing.
  *Mathematics of Operations Research,* vol 29, 837-860.
  - Ohlmann, J. and B. Thomas (2007).
  A compressed annealing approach to the traveling salesman problem with time windows.
  *INFORMS Journal on Computing,* vol 19, 80-90.

## Compatibility Matrix
| **Compiler** | **Version** | **Platform**  | **Operating System** | **Does It Work?** |
|:-------------|:-----------:|:-------------:|:--------------------:|:-----------------:|
| Intel C++    |   2025.4    |    x86_64     | Linux (Fedora 41)    |           Yes     |
| Intel C++    |   2025.4    |    x86_64     | Windows 11           |           Yes     |
| GNU C++      |     14      |    x86_64     | Linux (Fedora 41)    |           Yes     |
| GNU C++      |     14      | Apple Silicon | MacOS (Sequoia)      |           Yes     |
| GNU C++      |     14      | x86_64        | Windows 11           |           Yes     |
| Clang C++    |     16      | Apple Silicon | MacOS (Sequoia)      |           Yes     |
| Clang C++    |     19      |    x86_64     | Linux (Fedora 41)    |           Yes     |
| Clang C++    |     19      |    x86_64     | Windows 11           |           Yes     |
| Visual C++   |    2022     |    x86_64     | Windows 11           |           Yes     |

## Getting Djinni
The latest version of Djinni is always available from the 
[Releases page](https://github.com/rjhansen/djinni/releases).

## Building Djinni
There’s nothing *to* build: it’s just a couple of header files. That
doesn’t mean we’re leaving you helpless, though: there are two scripts
included to help you out. You will need [CMake](https://www.cmake.org)
installed and available on your system’s path in order to use them.

* **UNIX (including MacOS).** Go into the top of the Djinni directory,
  type `./build.sh`, and wait a little bit. If you have administrator
  privileges, the script will try to place Djinni in `/usr/local/include`.
  Without administrator privileges Djinni will be installed to the 
  `include` directory off your home folder.
* **Windows.** Go into the top of the Djinni directory, type `build.ps1`,
  and wait. The Djinni headers will be collected into a compressed folder
  for you, so that you may place them wherever you like for your project.

## Using Djinni

Djinni is an annealer with three pluggable mix-and-match components. They are

* **Worlds**. A `world` is a mathematical space defining a universe of
  possibilities.
* **Solutions.** A `solution` is a valid route through that mathematical
  space — but be careful: there’s no guarantee it’s an optimal one!
* **Penalties.** A `penalty` is something which causes the annealer to
  over time become less likely to accept inferior solutions, ultimately
  reaching a poit where it stabilizes on one solution because of the lack
  of superior nearby alternatives to explore.

Djinni ships with a world designed to represent instances of the Traveling
Salesman problem (`TravelingSalesmanWorld`), a solution to represent
solutions to that problem (`TravelingSalesmanSolution`), and two penalty
functions (`Compression` and `Simulated`).

Thus, our source code might look something like:

```c++
#include "djinni.h"
#include <iostream>

using edu::uiowa::tippie::djinni::TravelingSalesmanWorld;
using edu::uiowa::tippie::djinni::TravelingSalesmanSolution;
using edu::uiowa::tippie::djinni::Compression;
using edu::uiowa::tippie::djinni::Annealer;
using std::cout;
using std::endl;

int main()
{
  // Populate our world data from a Dumas dataset
  auto world = TravelingSalesmanWorld::loadFromDumasFile("Dumas-1.set");

  // Construct an initial random route through our solution space
  auto initial_solution = TravelingSalesmanSolution(world);

  // Craft our penalty function (parameters being the exponential
  // factor of compression, the pressure cap, and the fraction of
  // the pressure cap -- see the Ohlmann-Thomas paper for full details).
  auto penalty_function = Compression(0.06, 0.0, 0.9999);

  // Our annealer takes our penalty function, our initial solution,
  // and parameters denoting the:
  //
  // - temperature multiplier
  // - initial probability of accepting an inferior solution
  // - the terminal best iteration
  // - the minimum annealing iterations to apply before stabilizing
  // - the maximum annealing iterations to apply before stabilizing
  //
  // See the Ohlmann-Thomas paper for full details.
  auto annealer = Annealer(penalty_function,
                           initial_solution,
                           0.95,
                           0.94,
                           75,
                           100,
                           30000);
  
  // With all our pieces assembled, approximating a perfect solution
  // is as easy as hitting '.solve()' and dumping the annealer object
  // to the I/O stream of your choice.
  annealer.solve();
  cout << annealer << endl;
  return 0;
}
```

Upon running this with an appropriate dataset, you might see output like:

```json
{
	"best_solution": {
		"base_cost": 738,
		"penalty":   0
	}
	"best_iteration":          75,
	"iterations":              187,
	"count_limit":             30000,
	"minimum_iterations":      100,
	"sample_size":             10000,
	"multiplier":              0.95,
	"acceptance_probability":  0.94,
	"terminal_best_iteration": 75,
	"pressure":                237.777
}
```

This JSON output is ready to be ingested by the data pipeline of your choice.

## History
In the mid-2000s a pair of operations research professors at the
University of Iowa ([Barrett Thomas](https://tippie.uiowa.edu/people/barrett-thomas) 
and [Jeffrey Ohlmann](https://tippie.uiowa.edu/people/jeffrey-ohlmann)) came to me 
with a problem: they’d written a
[fine paper](https://myweb.uiowa.edu/bthoa/DownloadItems/TSPTWpaper4-05-05.pdf)
on a new kind of annealing algorithm but they couldn’t get their C++ 
implementation to live up to the theoretical promise. I was brought in 
as an early-career graduate student to reimplement the algorithm with 
an eye towards efficiency.

For Djinni’s twentieth anniversary I decided to dust off the old
code and refactor it somewhat to bring it up to the C++23 standard.
The code held up surprisingly well.

## Thanks go to…
(In no particular order)

* Tristan D. Thiede, my co-hacker on this way back when
* [Professor Jeffrey Ohlmann](https://tippie.uiowa.edu/people/jeffrey-ohlmann)
* [Professor Barrett Thomas](https://tippie.uiowa.edu/people/barrett-thomas)