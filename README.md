# Code Metrics Analyzer (CMA)

A command-line static analysis tool for C++ codebases. It scans a directory of `.cpp`/`.h` files, tokenizes and parses them without invoking a compiler, and reports size, structural, and complexity metrics (cyclomatic complexity, nesting depth, function/class counts, TODO markers, etc.).

Built in C++17 as a from-scratch lexer → parser → metrics → report pipeline (no external libraries).

## Quickstart

```bash
git clone https://github.com/<your-username>/CodeMetricsAnalyzer.git
cd CodeMetricsAnalyzer
cmake -B build
cmake --build build
```

## Usage

```bash
./build/cma <path> [--out report.txt]
```

Example — analyzing the project's own source:

```bash
./build/cma . --out report.txt
```

Sample output:

```
==================================
CODE METRICS REPORT
==================================
Files Analyzed : 15
Total Lines : 2295
Blank Lines : 423
Comments : 338
Functions : 44
Classes : 29
Variables : 121
Loops : 25
Conditions : 91
Maximum Nesting : 5
Cyclomatic Complexity : 221
Average Function Length : 18.3
Longest Function : walkTokens()
Length : 59 lines
TODO Comments : 6
==================================
```

## How it works

```
FileScanner  →  Lexer  →  Parser  →  MetricsEngine  →  ReportGenerator
 (discover      (source    (token       (per-file          (formatted
  + read files)  → tokens)  stream →     aggregation)        text output)
                             FileMetrics)
```

Five independent CMake static libraries (`cma_filesystem`, `cma_lexer`, `cma_parser`, `cma_metrics`, `cma_report`), each with a single responsibility, linked into one `cma` executable. Full design rationale, module-by-module code walkthrough, and complexity analysis are in [`CMA_Documentation.md`](CMA_Documentation.md).

## Known limitations

This is a portfolio project with a deliberately scoped-down feature set — these are documented tradeoffs, not oversights:

- Variable counting only recognizes primitive types (`int`, `double`, `auto`, ...) — `std::string`, `vector<T>`, and user-defined types aren't counted.
- Lambdas aren't counted as functions (the detection heuristic looks for `IDENTIFIER(...) {`, and lambdas start with `[`).
- Nested functions and local classes aren't tracked.
- No automated test suite yet (planned next).

See section 20 of `CMA_Documentation.md` for the complete list.

## Build requirements

- C++17 compiler (GCC 9+ or Clang recommended)
- CMake 3.16+

Compiles clean under `-Wall -Wextra -Wpedantic -Werror`.

## License

MIT — see [LICENSE](LICENSE).
