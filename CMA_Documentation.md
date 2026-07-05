# Code Metrics Analyzer (CMA)
## Technical Project Documentation

**Language:** C++17  
**Build System:** CMake 3.16+  
**Namespace:** `cma`  
**Total Source Lines:** ~1,337 across 13 files  
**Target Audience:** SDE-1 / Intern Portfolio

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Problem Statement](#2-problem-statement)
3. [Project Objectives](#3-project-objectives)
4. [Project Features](#4-project-features)
5. [Functional Requirements](#5-functional-requirements)
6. [Non-Functional Requirements](#6-non-functional-requirements)
7. [Complete Project Workflow](#7-complete-project-workflow)
8. [System Architecture](#8-system-architecture)
9. [Module-wise Description with Code Breakdown](#9-module-wise-description-with-code-breakdown)
10. [Folder Structure Explanation](#10-folder-structure-explanation)
11. [Data Flow](#11-data-flow)
12. [Algorithms Used](#12-algorithms-used)
13. [Data Structures Used](#13-data-structures-used)
14. [Design Patterns Used](#14-design-patterns-used)
15. [C++17 Concepts Used](#15-c17-concepts-used)
16. [Input and Output Flow](#16-input-and-output-flow)
17. [Error Handling Strategy](#17-error-handling-strategy)
18. [Performance Analysis](#18-performance-analysis)
19. [Scalability Analysis](#19-scalability-analysis)
20. [Limitations](#20-limitations)
21. [Future Enhancements](#21-future-enhancements)
22. [Real-World Use Cases](#22-real-world-use-cases)
23. [Technical Skills Demonstrated](#23-technical-skills-demonstrated)

---

## 1. Project Overview

The **Code Metrics Analyzer (CMA)** is a command-line static analysis tool written in C++17. It reads C++ source files from a target directory, processes them through a lexer-parser pipeline, computes quantitative software metrics, and produces a structured report — all without compiling or executing the code under analysis.

The tool analyzes C++ source and header files (`.cpp`, `.cc`, `.cxx`, `.c++`, `.h`, `.hpp`, `.hxx`, `.h++`) and reports metrics across three categories:

- **Size metrics:** total lines, blank lines, comment lines, source lines of code
- **Structural metrics:** functions, classes/structs/enums/namespaces, variables, `#include` directives
- **Complexity metrics:** cyclomatic complexity, nesting depth, loops, conditions, try/catch blocks, TODO/FIXME markers, average and longest function length

The project is designed as an intern/SDE-1 portfolio piece that demonstrates a complete, end-to-end pipeline in C++17 — from filesystem traversal through lexical analysis, parsing, metric aggregation, and report generation — with clean module boundaries, consistent error handling, and correct use of modern C++ idioms.

---

## 2. Problem Statement

Software development teams often need to understand the quality and complexity of a codebase at a glance. Questions like "how many functions does this project have?", "which functions are the longest?", "how complex is the control flow?", and "how many TODO items are outstanding?" require either manually reading all source files or purchasing expensive static analysis tools.

Lightweight, open tooling for these metrics typically requires a compiler toolchain (e.g., clang-tidy, cppcheck) or a language-specific runtime. This creates a barrier for quick, dependency-free metric snapshots on any C++ project.

CMA addresses this gap by providing a self-contained, zero-dependency C++17 binary that can analyze any C++ codebase purely through character-level tokenization and pattern matching — no compiler, no external libraries.

---

## 3. Project Objectives

The concrete objectives that shaped every design decision:

1. **Implement a complete static analysis pipeline** from file I/O through tokenization, parsing, metric aggregation, and report output — without external dependencies.
2. **Demonstrate clean module separation** where each stage (scan, read, lex, parse, aggregate, report) is an independent unit with a single, well-defined responsibility.
3. **Handle real-world C++ syntax robustly** — multi-line block comments, preprocessor directives with backslash-continuation, escape sequences in string literals, constructor initializer lists, and trailing return types.
4. **Report results deterministically** regardless of the underlying OS's filesystem iteration order.
5. **Fail gracefully** on unreadable files, permission errors, and malformed input — emitting a warning rather than crashing.
6. **Compile clean** under `-Wall -Wextra -Wpedantic -Werror` with no suppressions.

---

## 4. Project Features

All features listed here are implemented in the actual source code.

**File Discovery**
- Recursively scans a target directory or accepts a single file as input
- Recognizes 8 C++ file extensions: `.cpp`, `.cc`, `.cxx`, `.c++`, `.h`, `.hpp`, `.hxx`, `.h++`
- Sorts discovered paths for deterministic, reproducible ordering across platforms
- Skips permission-denied entries without crashing

**Lexical Analysis**
- Character-level state machine; no regular expressions used
- Recognizes 78 C++ keywords (including C++11/14/17 additions such as `constexpr`, `decltype`, `noexcept`, `nullptr`, `requires`, `consteval`, `constinit`)
- Emits 20 distinct token types covering keywords, identifiers, all literal kinds, delimiters, comments, preprocessor directives, operators, and structural markers
- Handles escape sequences in string and character literals
- Handles backslash-continuation in preprocessor directives
- Recovers gracefully from unterminated string literals and unclosed block comments

**Parsing and Metric Collection**
- Two-pass design: line classification pass followed by structural analysis pass
- Line classification with priority rule: CODE > COMMENT > BLANK (a mixed line is CODE)
- Block comments that span multiple lines mark every spanned line as COMMENT
- Function detection heuristic: `IDENTIFIER ( ... ) [specifiers] {` at brace depth 0 or 1
- Handles trailing specifiers between `)` and `{`: `const`, `noexcept`, `override`, `final`, `volatile`, `mutable`, `requires`, and `noexcept(expr)`
- Handles constructor initializer lists (`: member(val) { ... }`) and trailing return types (`-> RetType`)
- Cyclomatic complexity computed per file: base 1 + `if` + `switch` + `for` + `while` + `do` + `case` + `catch` + `&&` + `||` + `?`
- `&&` and `||` detected as two consecutive single-character `OPERATOR` tokens to avoid false positives on bitwise `&` and `|`

**Metrics Reported**
- Files analyzed
- Total lines, blank lines, comment lines
- Function count, longest function name and line count, average function length
- Class/struct/enum/namespace count
- Variable count (primitive-type declarations only — see Limitations)
- `#include` directive count
- Loop count (`for`, `while`, `do`)
- Condition count (`if`, `switch`)
- Try/catch block count
- Cyclomatic complexity (summed across all files)
- Maximum nesting depth (deepest single-file depth, not sum)
- TODO/FIXME comment marker count

**Output**
- Formatted console report to `stdout`
- Optional file output via `--out <filename>` flag (overwrites existing file)
- Console and file output share a single formatting function, so they can never diverge

---

## 5. Functional Requirements

These are requirements satisfied by the implementation:

| ID | Requirement | Satisfied by |
|----|-------------|-------------|
| FR-01 | Accept a directory path or single file path as input | `main.cpp` `parseArgs()`, `FileScanner::scan()` |
| FR-02 | Recursively discover all C++ source and header files | `FileScanner::scan()` with `recursive_directory_iterator` |
| FR-03 | Read full file content without requiring a compiler | `FileScanner::readFile()` via `std::ifstream` |
| FR-04 | Tokenize C++ source into a categorized token stream | `Lexer::tokenize()` |
| FR-05 | Classify each source line as blank, comment, or code | `Parser::classifyLines()` |
| FR-06 | Count structural elements: functions, classes, variables | `Parser::walkTokens()` |
| FR-07 | Compute cyclomatic complexity per file | `Parser::handleKeyword()` and `walkTokens()` OPERATOR case |
| FR-08 | Track function start/end lines and names | `tryBeginFunction()` / CLOSE_BRACE handler |
| FR-09 | Aggregate per-file metrics into project totals | `MetricsEngine::compute()` |
| FR-10 | Print a formatted report to console | `ReportGenerator::printSummary()` |
| FR-11 | Optionally save the same report to a file | `ReportGenerator::saveToFile()` |
| FR-12 | Detect TODO and FIXME markers in comments | `Parser::countTodos()` |
| FR-13 | Report the longest function and its line count | `MetricsEngine::compute()` inner function loop |
| FR-14 | Report maximum nesting depth across all files | `std::max` in `MetricsEngine::compute()` |

---

## 6. Non-Functional Requirements

| Category | Requirement | How It Is Met |
|----------|-------------|---------------|
| Portability | Must compile on GCC and Clang with C++17 | CMake sets `CMAKE_CXX_EXTENSIONS OFF` (no GNU extensions); GCC < 9 stdc++fs guard included |
| Warning hygiene | Zero warnings under `-Wall -Wextra -Wpedantic -Werror` | Enforced by CMake `add_compile_options` |
| Robustness | Never crash on bad input, bad paths, or malformed code | `std::error_code` paths, `std::optional` returns, graceful lexer recovery |
| Determinism | Same input always produces same output | `std::sort` on file list; no unordered iteration exposed to output |
| No external dependencies | Build requires only a C++17 compiler and CMake | Zero third-party libraries used |
| Maintainability | Each module can be modified without touching others | Five independent static libraries with explicit public interface headers |
| Testability | Individual modules can be linked in isolation | CMake `target_link_libraries(cma_parser PUBLIC cma_lexer)` chain enables per-module test executables |

---

## 7. Complete Project Workflow

```
User invokes: ./cma <path> [--out report.txt]
                          │
              ┌───────────▼───────────┐
              │    CLI Argument        │  parseArgs() in main.cpp
              │    Parsing             │  Produces Config{targetPath, outputFile}
              └───────────┬───────────┘
                          │
              ┌───────────▼───────────┐
              │    File Discovery      │  FileScanner::scan()
              │    (FileScanner)       │  recursive_directory_iterator,
              │                        │  extension filter, std::sort
              └───────────┬───────────┘
                          │  vector<filesystem::path>
              ┌───────────▼───────────┐
              │    For each file:      │
              │                        │
              │  ┌─────────────────┐   │
              │  │  File Read      │   │  FileScanner::readFile()
              │  │  (FileScanner)  │   │  rdbuf() bulk read → string
              │  └────────┬────────┘   │
              │           │ string     │
              │  ┌────────▼────────┐   │
              │  │   Lexer         │   │  Lexer::tokenize()
              │  │                 │   │  character state machine →
              │  │                 │   │  vector<Token> (20 types)
              │  └────────┬────────┘   │
              │           │ tokens     │
              │  ┌────────▼────────┐   │
              │  │   Parser        │   │  Parser::analyze()
              │  │  Pass 1: lines  │   │  classifyLines() → LineType map
              │  │  Pass 2: walk   │   │  walkTokens() → FileMetrics
              │  └────────┬────────┘   │
              │           │ FileMetrics│
              │  ┌────────▼────────┐   │
              │  │  MetricsEngine  │   │  engine.addFile(path, fm)
              │  │  .addFile()     │   │
              │  └─────────────────┘   │
              └───────────┬───────────┘
                          │  (loop complete)
              ┌───────────▼───────────┐
              │  MetricsEngine        │  engine.compute()
              │  .compute()           │  Single pass over m_files,
              │                        │  aggregates ProjectMetrics
              └───────────┬───────────┘
                          │  ProjectMetrics
              ┌───────────▼───────────┐
              │  ReportGenerator      │  printSummary() → stdout
              │                        │  saveToFile()  → file (if --out)
              └───────────────────────┘
```

---

## 8. System Architecture

### Layered View

```
┌────────────────────────────────────────────┐
│  CLI Layer          main.cpp               │  Argument parsing, pipeline orchestration
├────────────────────────────────────────────┤
│  Output Layer       ReportGenerator        │  Formatting, console/file output
├────────────────────────────────────────────┤
│  Aggregation Layer  MetricsEngine          │  Cross-file metric aggregation
├────────────────────────────────────────────┤
│  Analysis Layer     Parser                 │  Structural analysis, metric collection
├────────────────────────────────────────────┤
│  Tokenization Layer Lexer                  │  Character-to-token transformation
├────────────────────────────────────────────┤
│  I/O Layer          FileScanner            │  Directory traversal, file reading
└────────────────────────────────────────────┘
```

Dependencies flow strictly **downward**: no lower layer knows about any layer above it. `main.cpp` is the only module that touches all layers.

### Module Dependency Graph

```
main.cpp
  ├── cma_filesystem   (FileScanner)
  ├── cma_lexer        (Lexer, Token)
  ├── cma_parser       (Parser, ParseResult)  ──depends on──►  cma_lexer
  ├── cma_metrics      (MetricsEngine)         ──depends on──►  cma_parser
  └── cma_report       (ReportGenerator)       ──depends on──►  cma_metrics
```

This is the exact dependency chain encoded in `CMakeLists.txt` via `target_link_libraries`. Each library only declares upward dependencies through its `PUBLIC` header includes; `main.cpp` links against all five to orchestrate the full pipeline.

### Data Flow Between Modules (Types)

```
FileScanner::scan()     → vector<filesystem::path>
FileScanner::readFile() → optional<string>
Lexer::tokenize()       → vector<Token>
Parser::analyze()       → FileMetrics
MetricsEngine::compute()→ ProjectMetrics
ReportGenerator         → (stdout / file)
```

Every stage output is a concrete value type — no shared pointers, no inheritance, no virtual dispatch in the data-passing layer.

---

## 9. Module-wise Description with Code Breakdown

---

### 9.1 `Token.h` — Token Taxonomy

**File:** `include/lexer/Token.h`  
**Responsibility:** Define the vocabulary that the Lexer produces and the Parser consumes.

**`enum class TokenType`** — 20 values, each representing one category of C++ lexical element:

```
KEYWORD         — recognized C++ keyword (auto, if, class, …)
IDENTIFIER      — user-defined name (variable, function, type)
STRING_LITERAL  — "…"    (content treated as opaque)
CHAR_LITERAL    — '…'    (content treated as opaque)
NUMBER_LITERAL  — 42, 3.14, 0xFF, 1.5e-3
OPEN_BRACE      — {
CLOSE_BRACE     — }
OPEN_PAREN      — (
CLOSE_PAREN     — )
OPEN_BRACKET    — [
CLOSE_BRACKET   — ]
SEMICOLON       — ;
LINE_COMMENT    — // … (up to but not including '\n')
BLOCK_COMMENT   — /* … */ (including newlines inside)
PREPROCESSOR    — # … (including backslash-continuation lines)
OPERATOR        — + - * / % = < > ! & | ^ ~ ? : .
PUNCTUATION     — , @ and any unrecognized single character
NEWLINE         — explicit '\n' token
END_OF_FILE     — sentinel
UNKNOWN         — default; never emitted by the Lexer intentionally
```

The reason delimiters (`{`, `}`, `(`, `)`, `[`, `]`, `;`) each have their own type rather than being `OPERATOR` or `PUNCTUATION` is so the Parser can match them without re-reading the `value` string — it compares `tok.type == TokenType::OPEN_BRACE`, which is a single integer comparison.

**`struct Token`** — four fields:
```cpp
TokenType   type  = TokenType::UNKNOWN;  // discriminator
std::string value;                        // raw text from source
int         line  = 0;                    // 1-based source line number
int         col   = 0;                    // 1-based column number
```
`line` and `col` are set at lex time and never modified. The Parser uses `line` to populate `FunctionInfo::startLine`/`endLine` and for line classification.

**Inline predicate functions** — four free functions defined inline in the header so any translation unit that includes `Token.h` can use them without a linking dependency on a `.cpp`:

```cpp
isLoopKeyword(tok)  — true for  for / while / do
isCondKeyword(tok)  — true for  if / switch
isClassLike(tok)    — true for  class / struct / enum / namespace
isExceptionKw(tok)  — true for  try / catch
```

These predicates are used in `Parser::handleKeyword()` to dispatch to the appropriate counter without a chain of string comparisons.

---

### 9.2 `Lexer.h` / `Lexer.cpp` — Character-Level Tokenizer

**Files:** `include/lexer/Lexer.h`, `src/lexer/Lexer.cpp`  
**Responsibility:** Transform a `const std::string&` of raw C++ source text into a `std::vector<Token>`.

#### Class Interface

```cpp
class Lexer {
public:
    explicit Lexer(const std::string& source);
    [[nodiscard]] std::vector<Token> tokenize();
private:
    // 8 category-specific lexers
    Token lexIdentifierOrKeyword();
    Token lexNumber();
    Token lexString();
    Token lexCharLiteral();
    Token lexLineComment();
    Token lexBlockComment();
    Token lexPreprocessor();
    Token lexSymbol();
    // 4 stream primitives
    char cur()    const noexcept;
    char peekAt(int offset = 1) const noexcept;
    char advance() noexcept;
    bool atEnd()  const noexcept;
    bool isKeyword(const std::string& word) const noexcept;

    const std::string& m_src;
    std::size_t        m_pos  = 0;
    int                m_line = 1;
    int                m_col  = 1;
};
```

The constructor takes `const std::string&` — it stores a reference, not a copy. The string must outlive the Lexer instance (guaranteed in `main.cpp` where both live in the same loop body).

#### Keyword Table (Lexer.cpp, file scope)

```cpp
static const std::unordered_set<std::string> kKeywords = { /* 78 keywords */ };
```

`static` at file scope gives internal linkage — the symbol is private to `Lexer.cpp`. `unordered_set<string>` gives O(1) average lookup, which matters because `isKeyword()` is called for every identifier token. The table includes C++11 through C++17 additions: `constexpr`, `decltype`, `noexcept`, `nullptr`, `thread_local`, `static_assert`, `char16_t`, `char32_t`, and `requires`.

`final` and `override` are included even though they are context-sensitive identifiers in the C++ standard (not true keywords). Including them ensures they are never misidentified as user-defined function or variable names, which keeps the function detection heuristic clean.

#### `tokenize()` — Main Loop Line-by-Line

```cpp
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(m_src.size() / 4);  // (1)
```
**(1)** The reserve heuristic estimates roughly one token per 4 source characters. This avoids repeated `vector` reallocations for large files. It is a guess, not a guarantee.

```cpp
    while (!atEnd()) {
        const char c = cur();

        if (c == '\n') {
            tokens.push_back({TokenType::NEWLINE, "\n", m_line, m_col});
            advance();
            continue;
        }  // (2)
```
**(2)** Newlines are emitted as explicit `NEWLINE` tokens. This is the key design decision that lets the Parser reconstruct line numbers without a separate line-number map. Every other token records its own `line` at lex time; NEWLINE tokens serve as line-boundary markers when the Parser walks the token vector in Pass 2.

```cpp
        if (std::isspace(static_cast<unsigned char>(c))) { advance(); continue; }
```
All non-newline whitespace (spaces, tabs, carriage returns) is consumed and discarded. `static_cast<unsigned char>` is required before passing to `std::isspace` to avoid undefined behavior on platforms where `char` is signed and the value is negative (characters above ASCII 127).

```cpp
        if (c == '/' && peekAt() == '/') { tokens.push_back(lexLineComment());  continue; }
        if (c == '/' && peekAt() == '*') { tokens.push_back(lexBlockComment()); continue; }
```
Two-character lookahead: `peekAt()` reads `m_src[m_pos + 1]` without consuming it. The `//` check must appear before any single-character `/` dispatch to correctly route comments.

```cpp
        if (c == '#') { tokens.push_back(lexPreprocessor()); continue; }
        if (c == '"') { tokens.push_back(lexString());       continue; }
        if (c == '\'') { tokens.push_back(lexCharLiteral()); continue; }
        if (std::isdigit(...)) { tokens.push_back(lexNumber()); continue; }
        if (std::isalpha(...) || c == '_') { tokens.push_back(lexIdentifierOrKeyword()); continue; }
        tokens.push_back(lexSymbol());
    }
    tokens.push_back({TokenType::END_OF_FILE, "", m_line, m_col});
    return tokens;
}
```
The `END_OF_FILE` sentinel at the end prevents the Parser from reading past the end of the vector; it serves as a guaranteed stopping condition in scanning loops.

#### `lexIdentifierOrKeyword()`
Consumes alphanumeric characters and underscores, then checks the result against `kKeywords`. Returns `KEYWORD` or `IDENTIFIER`. The keyword vs. identifier distinction is deferred to the end so the full word is available for the hash lookup.

#### `lexNumber()`
Handles integers, hex literals (`0xFF` — `isalnum` absorbs `x` and hex digits), floats (`3.14`), and scientific notation (`1.5e-3` — the `e`/`E` check allows a following `+` or `-`). It does **not** validate that the resulting string is a well-formed numeric literal; that is not CMA's job.

#### `lexString()`
Consumes from opening `"` to closing `"`, handling `\"` escape sequences. If a `\n` is encountered before the closing `"`, the literal is treated as unterminated and the function returns what it has consumed. No exception is thrown. The string interior is carried in `value` but the Parser never reads it.

#### `lexCharLiteral()`
Same structure as `lexString()` but for `'…'`. Handles `'\n'`, `'\\'`, `'\''` via the same backslash-skip logic.

#### `lexLineComment()`
Consumes from `//` to (but not including) the `\n`. The `\n` is left unconsumed so the main loop will emit it as a `NEWLINE` token, preserving the correct line count for the next token.

#### `lexBlockComment()`
Consumes from `/*` to `*/`, including embedded newlines. Because `advance()` updates `m_line` every time it sees `\n`, the line counter stays correct throughout a multi-line block comment. An unclosed block comment (reaching `atEnd()` before `*/`) emits whatever was consumed — no exception, no partial-token corruption.

#### `lexPreprocessor()`
Consumes from `#` to the end of the logical line. A backslash immediately before `\n` is a line continuation — the function replaces the `\` with a space and consumes the `\n`, then continues on the next line. This means a multi-line macro like:
```cpp
#define MAX(a, b) \
    ((a) > (b) ? (a) : (b))
```
is emitted as a single `PREPROCESSOR` token.

#### `lexSymbol()`
Handles all remaining single characters. Structural delimiters (`{`, `}`, `(`, `)`, `[`, `]`, `;`) each return their dedicated `TokenType`. Operator characters (`+`, `-`, `*`, etc., including `&`, `|`, `?`, `:`) return `OPERATOR` with a single-character value. Every other character returns `PUNCTUATION`.

The decision to emit `&` and `|` as single-character `OPERATOR` tokens (rather than combining `&&` and `||` in the lexer) is deliberate: it lets the Parser detect `&&` as two consecutive identical `OPERATOR` tokens. This avoids the lexer needing to distinguish `&` (bitwise AND), `&&` (logical AND), and `&=` (compound assignment), which would require more lookahead logic.

#### Character Stream Primitives

```cpp
char Lexer::cur() const noexcept  { return atEnd() ? '\0' : m_src[m_pos]; }
char Lexer::peekAt(int offset) const noexcept {
    const auto idx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(m_pos) + offset);
    return (idx < m_src.size()) ? m_src[idx] : '\0';
}
char Lexer::advance() noexcept {
    const char c = m_src[m_pos++];
    if (c == '\n') { ++m_line; m_col = 1; }
    else           { ++m_col; }
    return c;
}
bool Lexer::atEnd() const noexcept { return m_pos >= m_src.size(); }
```

`peekAt()` casts through `std::ptrdiff_t` before converting back to `std::size_t`. This is defensive: if `offset` were negative and `m_pos` were 0, the arithmetic would wrap silently on unsigned types. The cast sequence preserves the signed semantics during the offset calculation.

`advance()` is the only place `m_line` and `m_col` change. Since every character path goes through `advance()`, line/column tracking is always consistent.

---

### 9.3 `FileScanner.h` / `FileScanner.cpp` — File Discovery and Reading

**Files:** `include/filesystem/FileScanner.h`, `src/filesystem/FileScanner.cpp`  
**Responsibility:** Discover C++ files in a directory tree; read individual file contents.

This module is deliberately split from the Lexer: it performs I/O only. It has no knowledge of tokens, lines, or metrics.

#### Extension Set (Anonymous Namespace)

```cpp
namespace {
const std::unordered_set<std::string> kCppExtensions = {
    ".cpp", ".cc", ".cxx", ".c++",
    ".h",   ".hpp", ".hxx", ".h++"
};
} // anonymous namespace
```

The anonymous namespace gives `kCppExtensions` internal linkage — it is invisible outside `FileScanner.cpp`. This prevents linker collisions if another translation unit also defines a constant named `kCppExtensions`. `unordered_set` is chosen over `vector` because the only operation ever performed is membership testing (`count()`), which is O(1) on an unordered set vs. O(n) on a vector.

#### Constructor

```cpp
FileScanner::FileScanner(std::filesystem::path rootPath)
    : m_rootPath(std::move(rootPath)) {}
```

Takes `rootPath` by value then moves it into `m_rootPath`. This avoids a copy when the caller passes a temporary. If the caller passes an lvalue, one copy is made (at the call site, into the parameter); the subsequent move is then free.

#### `scan()` — Line-by-Line

```cpp
std::vector<std::filesystem::path> FileScanner::scan() const {
    std::vector<std::filesystem::path> files;
    std::error_code ec;
    if (!std::filesystem::exists(m_rootPath, ec) || ec) {
        std::cerr << "[FileScanner] Path does not exist: " << m_rootPath << '\n';
        return files;
    }
```
The `std::error_code` overload of `exists()` does not throw — it sets `ec` on failure. This is important on Windows where a UNC path or network share that doesn't respond would otherwise throw a `filesystem_error`. Checking both `!exists()` and `ec` catches the case where `exists()` itself reports an error.

```cpp
    if (std::filesystem::is_regular_file(m_rootPath)) {
        if (isCppFile(m_rootPath)) files.push_back(m_rootPath);
        else std::cerr << "[FileScanner] File is not a recognised C++ source: " << m_rootPath << '\n';
        return files;
    }
```
Single-file shortcut. If the user passes a specific file (e.g., `./cma src/main.cpp`), directory traversal is skipped entirely.

```cpp
    try {
        const auto opts = std::filesystem::directory_options::skip_permission_denied;
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(m_rootPath, opts)) {
            if (entry.is_regular_file() && isCppFile(entry.path()))
                files.push_back(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[FileScanner] Filesystem error during scan: " << e.what() << '\n';
    }
    std::sort(files.begin(), files.end());
    return files;
}
```
`skip_permission_denied` is a directory-level flag: when the iterator encounters a subdirectory it cannot enter, it silently skips to the next entry rather than throwing. A `try/catch` around the loop catches any other `filesystem_error` (e.g., a hardware I/O error mid-traversal) and returns partial results rather than propagating the exception to the caller. `std::sort` on `filesystem::path` uses lexicographic ordering, which gives deterministic output regardless of which operating system or filesystem is in use.

#### `readFile()` — Line-by-Line

```cpp
std::optional<std::string> FileScanner::readFile(const std::filesystem::path& filePath) {
    std::ifstream stream(filePath, std::ios::in);
    if (!stream.is_open()) {
        std::cerr << "[FileScanner] Cannot open: " << filePath << '\n';
        return std::nullopt;
    }
    std::ostringstream buf;
    buf << stream.rdbuf();
    if (stream.bad()) {
        std::cerr << "[FileScanner] I/O error reading: " << filePath << '\n';
        return std::nullopt;
    }
    return buf.str();
}
```

`buf << stream.rdbuf()` reads the entire file in a single stream operation through the stream buffer's internal buffer. This is faster than a `getline()` loop because it avoids repeatedly constructing and appending to individual `std::string` objects.

The function checks `stream.bad()` (non-recoverable hardware/OS error) but deliberately does **not** check `stream.fail()`. After `rdbuf()` reads to end-of-file, some standard library implementations set the `failbit` in addition to `eofbit`, because `eofbit` implies a failed extraction. Checking `fail()` would incorrectly report an error on a successfully read file.

`static` declaration: `readFile()` has no instance state — it is a pure path-to-content transformation. Making it static documents this contract and allows callers to call it without an instance.

`[[nodiscard]]` on both `scan()` and `readFile()`: if the caller discards the return value, the compiler emits a warning. This catches bugs where a caller forgets to use the results.

---

### 9.4 `ParseResult.h` — Data Structures for Analysis Results

**File:** `include/parser/ParseResult.h`  
**Responsibility:** Define the data types that the Parser produces and the MetricsEngine consumes.

#### `FunctionInfo`

```cpp
struct FunctionInfo {
    std::string name;       // identifier token value (e.g., "processOrder")
    int         startLine = 0;  // line of the IDENTIFIER token
    int         endLine   = 0;  // line of the matching CLOSE_BRACE
    [[nodiscard]] int lineCount() const noexcept {
        return (endLine >= startLine) ? (endLine - startLine + 1) : 0;
    }
};
```

`lineCount()` returns 0 if `endLine < startLine`, which is a safety check for the case where a function's closing brace was never found (e.g., malformed input). The `+1` is because both start and end lines are inclusive.

`longestFunction()` and `avgFunctionLength()` are declared in `ParseResult.h` but **defined in `Parser.cpp`** — not in the header. This is because they use `std::max_element` with a lambda that requires `FunctionInfo` to be a complete type. Placing them in `Parser.cpp` ensures this is always true when the TU is compiled.

#### `ClassInfo`

```cpp
struct ClassInfo {
    enum class Kind { CLASS, STRUCT, ENUM, NAMESPACE };
    std::string name;
    int         line = 0;
    Kind        kind = Kind::CLASS;
};
```

All four kinds (`class`, `struct`, `enum`, `namespace`) are tracked in a single `classes` vector in `FileMetrics`. The MetricsEngine aggregates the total count across all kinds into `ProjectMetrics::classCount`. The `Kind` enum is not exposed in the final report — it exists for potential future use (e.g., reporting class count vs. namespace count separately).

#### `FileMetrics`

```cpp
struct FileMetrics {
    int totalLines, blankLines, commentLines, codeLines;
    std::vector<FunctionInfo> functions;
    std::vector<ClassInfo>    classes;
    int includeCount, variableCount;
    int loopCount, conditionCount, tryCatchCount;
    int maxNestingDepth;
    int cyclomaticComplexity = 1;  // starts at 1 (base path)
    int todoCount;
    // derived helpers
    int functionCount() const noexcept;
    int classCount()    const noexcept;
    const FunctionInfo* longestFunction()  const noexcept;
    double              avgFunctionLength() const noexcept;
};
```

`cyclomaticComplexity` is initialized to `1`, not `0`. McCabe's cyclomatic complexity formula is: M = E − N + 2P, which simplifies to 1 + (number of decision points) for a single connected component. Each `if`, `while`, `for`, `do`, `switch`, `case`, `catch`, `&&`, `||`, and `?` adds 1.

`functions` and `classes` are stored as `std::vector` inside the struct. When `engine.addFile()` is called with `std::move(fm)`, the vectors move-construct into the stored copy at O(1) cost rather than deep-copying all `FunctionInfo` and `ClassInfo` elements.

---

### 9.5 `Parser.h` / `Parser.cpp` — Two-Pass Source Analyzer

**Files:** `include/parser/Parser.h`, `src/parser/Parser.cpp`  
**Responsibility:** Transform a `vector<Token>` into a `FileMetrics` struct via two sequential passes over the token stream.

#### Constructor

```cpp
Parser::Parser(const std::vector<Token>& tokens, int totalLines)
    : m_tokens(tokens), m_totalLines(totalLines)
{
    m_lineTypes.assign(static_cast<std::size_t>(totalLines), LineType::BLANK);
    m_result.totalLines = totalLines;
}
```

`m_lineTypes` is pre-filled with `BLANK` for every line. The classification pass then upgrades lines that contain tokens — it never downgrades. The `totalLines` value comes from `main.cpp` which counts `'\n'` characters in the raw source and adds 1 to account for files without a trailing newline.

#### `analyze()` — Public Entry Point

```cpp
FileMetrics Parser::analyze() {
    classifyLines();  // Pass 1
    walkTokens();     // Pass 2
    for (const auto& lt : m_lineTypes) {
        switch (lt) {
            case LineType::BLANK:   ++m_result.blankLines;   break;
            case LineType::COMMENT: ++m_result.commentLines; break;
            case LineType::CODE:    ++m_result.codeLines;    break;
        }
    }
    m_result.maxNestingDepth = m_maxDepth;
    return m_result;
}
```

The tally loop runs after both passes so that the final counts reflect the full classification (a line that starts as COMMENT but later gets upgraded to CODE by a non-comment token will be counted as CODE). `m_maxDepth` is a running maximum maintained during Pass 2.

#### Pass 1: `classifyLines()`

```cpp
void Parser::classifyLines() {
    const auto mark = [&](int srcLine, LineType lt) {
        const auto idx = static_cast<std::size_t>(srcLine - 1);
        if (idx >= m_lineTypes.size()) return;   // (1)
        if (lt == LineType::CODE ||              // (2)
            (lt == LineType::COMMENT && m_lineTypes[idx] == LineType::BLANK)) {
            m_lineTypes[idx] = lt;
        }
    };
```

**(1)** Bounds check: if a token claims to be on a line beyond `totalLines` (possible with malformed source), the lambda silently ignores it rather than causing undefined behavior via out-of-bounds access.

**(2)** The upgrade-only rule: CODE can always upgrade; COMMENT can only upgrade from BLANK (not from CODE). This handles mixed lines like `int x = 0; // initialized to zero` — the comment token arrives before the code token for that line, marks it COMMENT, but when the `int` keyword token arrives, it upgrades the line to CODE. A line is never downgraded.

```cpp
    for (const auto& tok : m_tokens) {
        if (tok.type == TokenType::END_OF_FILE ||
            tok.type == TokenType::NEWLINE) continue;

        if (tok.type == TokenType::LINE_COMMENT) {
            mark(tok.line, LineType::COMMENT);
            m_result.todoCount += countTodos(tok.value);

        } else if (tok.type == TokenType::BLOCK_COMMENT) {
            int line = tok.line;
            for (char c : tok.value) {           // (3)
                mark(line, LineType::COMMENT);
                if (c == '\n') ++line;
            }
            m_result.todoCount += countTodos(tok.value);

        } else if (tok.type == TokenType::PREPROCESSOR) {
            mark(tok.line, LineType::CODE);
            if (tok.value.find("#include") != std::string::npos)
                ++m_result.includeCount;        // (4)

        } else {
            mark(tok.line, LineType::CODE);
        }
    }
}
```

**(3)** Block comment spanning: the function walks the raw text of the block comment token character by character, calling `mark(line, COMMENT)` for each line and incrementing `line` at every `\n`. This correctly marks every line inside a multi-line `/* ... */` comment without requiring a second scan of the source text.

**(4)** `#include` counting by substring search within the preprocessor token's value. The Lexer stores the full directive text (e.g., `#include <vector>`), so a simple `find("#include")` is sufficient.

#### Pass 2: `walkTokens()` — Switch-Based Dispatch

```cpp
void Parser::walkTokens() {
    const std::size_t n = m_tokens.size();
    for (std::size_t i = 0; i < n; ++i) {
        const Token& tok = m_tokens[i];
        switch (tok.type) {

        case TokenType::OPEN_BRACE:
            ++m_braceDepth;
            m_maxDepth = std::max(m_maxDepth, m_braceDepth);
            break;

        case TokenType::CLOSE_BRACE:
            if (m_inFunction && m_braceDepth == m_fnBraceDepth) {  // (5)
                m_currentFn.endLine = tok.line;
                m_result.functions.push_back(m_currentFn);
                m_inFunction = false;
            }
            if (m_braceDepth > 0) --m_braceDepth;
            break;
```

**(5)** Function closure is detected **before** decrementing `m_braceDepth`. When the closing brace of a function body is found, `m_braceDepth` still equals `m_fnBraceDepth`. After the check, `m_braceDepth` is decremented. This ordering is critical — decrementing first would mean `m_braceDepth` is one less than `m_fnBraceDepth` and the check would never fire.

```cpp
        case TokenType::KEYWORD:
            handleKeyword(i);
            break;

        case TokenType::IDENTIFIER:
            if (i + 1 < n && m_tokens[i + 1].type == TokenType::OPEN_PAREN)
                tryBeginFunction(i);
            else
                handleVariableDecl(i);
            break;

        case TokenType::OPERATOR:
            if (tok.value == "?") {
                ++m_result.cyclomaticComplexity;
            } else if ((tok.value == "&" || tok.value == "|") &&
                        i + 1 < n &&
                        m_tokens[i + 1].type  == TokenType::OPERATOR &&
                        m_tokens[i + 1].value == tok.value) {
                ++m_result.cyclomaticComplexity;
                ++i;   // (6)
            }
            break;
```

**(6)** When `&&` or `||` is detected (two consecutive identical single-character OPERATOR tokens), the loop index `i` is incremented to skip the second character. Without this skip, the second `&` or `|` would be processed in the next iteration and trigger another false match.

#### `handleKeyword()`

Dispatches based on keyword value:
- `isLoopKeyword()` → increments `loopCount` and `cyclomaticComplexity`
- `isCondKeyword()` → increments `conditionCount` and `cyclomaticComplexity`
- `"case"` → increments only `cyclomaticComplexity` (each case label is an independent path; `switch` itself is counted via `isCondKeyword`)
- `"catch"` → increments `cyclomaticComplexity`
- `"try"` → increments `tryCatchCount`
- `isClassLike()` → records a `ClassInfo` using the identifier token at `idx + 1`

#### `tryBeginFunction()` — Function Detection Heuristic

```cpp
void Parser::tryBeginFunction(std::size_t identIdx) {
    if (m_inFunction || m_braceDepth > 1) return;   // (7)

    const std::size_t closeParenIdx = findMatchingParen(identIdx + 1);
    if (closeParenIdx >= m_tokens.size()) return;

    const std::size_t bodyIdx = skipTrailingSpecifiers(closeParenIdx + 1);
    if (bodyIdx >= m_tokens.size()) return;
    if (m_tokens[bodyIdx].type != TokenType::OPEN_BRACE) return;   // (8)

    m_inFunction   = true;
    m_fnBraceDepth = m_braceDepth + 1;
    m_currentFn.name      = m_tokens[identIdx].value;
    m_currentFn.startLine = m_tokens[identIdx].line;
}
```

**(7)** Two early exits: (a) we are already tracking a function — nesting is not supported; (b) brace depth > 1 means we are inside a nested scope (e.g., inside a function body or a class that is itself inside a namespace with depth > 0). The heuristic only recognizes functions at depth 0 (global) or depth 1 (first level inside a class/namespace).

**(8)** If the token after `)` (and specifiers) is not `{`, this is a function declaration (e.g., `void foo();`) rather than a definition. Declarations are not counted.

The pattern being detected:
```
IDENTIFIER  OPEN_PAREN  ...  CLOSE_PAREN  [specifiers]  OPEN_BRACE
```
Lambdas (`[captures](...){...}`) are excluded because the token immediately before `(` in the heuristic check is an `IDENTIFIER`, and lambda call operators start with `[` — they are never matched.

#### `skipTrailingSpecifiers()`

Handles specifiers that can appear between `)` and `{` in C++ function signatures:

- Keywords: `const`, `noexcept`, `override`, `final`, `volatile`, `mutable`, `requires`
- `noexcept(expr)` — tracked with an internal `parenDepth` counter so the expression inside `(...)` is not misinterpreted
- Constructor initializer list `": member(val), ..."` — detected by a lone `:` operator; fast-forwards to the next `{`
- Trailing return type `"-> RetType"` — detected by `-` followed by `>`; fast-forwards to the next `{` or `;`

#### `handleVariableDecl()` — Variable Counting Heuristic

```cpp
static const std::unordered_set<std::string> kPrimitiveTypes = {
    "int","long","short","char","bool","float","double",
    "unsigned","signed","auto","wchar_t","size_t","void"
};
```

A variable is counted when an `IDENTIFIER` token is immediately preceded (walking backward past `*`, `&`, and optional `const`) by one of the 13 primitive type keywords, and the token after the identifier is not `(` (which would make it a function).

This is intentionally approximate. `std::string s`, `vector<int> v`, and user-defined type declarations are **not** counted. This limitation is explicitly documented in `ParseResult.h` and acknowledged in `Parser.h`.

#### `countTodos()`

```cpp
int Parser::countTodos(const std::string& text) const {
    int count = 0;
    for (std::size_t pos = 0;
         (pos = text.find("TODO", pos)) != std::string::npos; pos += 4) ++count;
    for (std::size_t pos = 0;
         (pos = text.find("FIXME", pos)) != std::string::npos; pos += 5) ++count;
    return count;
}
```

Two separate linear scans with `std::string::find`. Called on each `LINE_COMMENT` and `BLOCK_COMMENT` token's `value` string. Case-sensitive: `todo` or `fixme` (lowercase) are not counted.

---

### 9.6 `Metrics.h` — Project-Level Aggregated Results

**File:** `include/metrics/Metrics.h`  
**Responsibility:** Define the `ProjectMetrics` struct that holds the aggregated view of the entire project.

`ProjectMetrics` is a plain aggregate struct with all fields zero/empty-initialized by default. There is no logic in this file — it is purely a data container. Key design notes:

- **`cyclomaticComplexity`** is a **sum** across all files (each file starts at 1, so a project with N files has a baseline of N).
- **`maxNestingDepth`** is **not** a sum — it is the single deepest nesting level observed in any one file. Summing per-file nesting depths would be meaningless.
- **`avgFunctionLength`** is computed by `MetricsEngine::compute()` as total function lines divided by total function count (not an average of per-file averages, which would be statistically incorrect for files with varying numbers of functions).
- **`longestFunctionName`** is stored as `"functionName()"` — the parentheses are appended by `MetricsEngine::compute()` for display clarity.

---

### 9.7 `MetricsEngine.cpp` — Aggregation Engine

**Files:** `include/metrics/MetricsEngine.h` (inferred), `src/metrics/MetricsEngine.cpp`  
**Responsibility:** Accumulate per-file `FileMetrics` results and produce a single `ProjectMetrics`.

#### Storage

```cpp
std::vector<std::pair<std::string, FileMetrics>> m_files;
```

Each entry is a `(filename, metrics)` pair. The filename (from `filepath.string()` in `main.cpp`) is stored for diagnostic purposes and potential future per-file reporting. The `FileMetrics` is stored by value; when `addFile()` is called with `std::move(fm)`, the stored copy is move-constructed.

#### `compute()` — Aggregation Pass

```cpp
ProjectMetrics MetricsEngine::compute() const {
    ProjectMetrics pm;
    pm.filesAnalyzed = static_cast<int>(m_files.size());

    long totalFnLength = 0;
    int  totalFnCount  = 0;

    for (const auto& [filename, fm] : m_files) {   // (9)
        pm.totalLines        += fm.totalLines;
        pm.blankLines        += fm.blankLines;
        // ... (all additive metrics)
        pm.maxNestingDepth    = std::max(pm.maxNestingDepth, fm.maxNestingDepth);  // (10)

        for (const auto& fn : fm.functions) {
            totalFnLength += fn.lineCount();
            ++totalFnCount;
            if (fn.lineCount() > pm.longestFunctionLines) {
                pm.longestFunctionLines = fn.lineCount();
                pm.longestFunctionName  = fn.name + "()";
            }
        }
    }
    pm.avgFunctionLength = (totalFnCount > 0)
        ? static_cast<double>(totalFnLength) / totalFnCount : 0.0;
    return pm;
}
```

**(9)** C++17 structured binding: `const auto& [filename, fm]` unpacks the `pair<string, FileMetrics>` directly. This is equivalent to `const auto& p = m_files[i]; const auto& filename = p.first; const auto& fm = p.second;` but more readable.

**(10)** `std::max` rather than summation for nesting depth: depth is a depth measurement per file, not an additive quantity. The project-level value is the worst-case depth seen in any single file.

`compute()` is `const` — calling it does not modify the engine's state, and can be called multiple times safely.

---

### 9.8 `ReportGenerator.h` / `ReportGenerator.cpp` — Output Formatter

**Files:** `include/report/ReportGenerator.h`, `src/report/ReportGenerator.cpp`  
**Responsibility:** Format a `ProjectMetrics` struct as human-readable text for console or file output.

#### Static Class Design

```cpp
class ReportGenerator {
public:
    static void printSummary(const ProjectMetrics& metrics, std::ostream& out);
    [[nodiscard]] static bool saveToFile(const ProjectMetrics& metrics,
                                          const std::string& outputPath);
private:
    static void writeReport(const ProjectMetrics& metrics, std::ostream& out);
};
```

All methods are `static` because `ReportGenerator` has no instance state whatsoever. It is a namespace with a class name. This design makes it impossible to accidentally instantiate a `ReportGenerator` object and holds open a slot for a future constructor-injectable formatter if needed.

#### Anonymous Namespace Constant

```cpp
namespace {
    constexpr char kSeparator[] = "==================================";
}
```

`constexpr` at namespace scope in an anonymous namespace: compile-time constant, internal linkage, zero runtime overhead. The anonymous namespace prevents `kSeparator` from conflicting with any identically named symbol in another translation unit.

#### `writeReport()` — Single Source of Truth

```cpp
void ReportGenerator::writeReport(const ProjectMetrics& m, std::ostream& out) {
    out << kSeparator << '\n';
    out << "CODE METRICS REPORT\n";
    out << kSeparator << '\n';
    // ... all metric lines ...
    out << "Average Function Length : "
        << std::fixed << std::setprecision(1) << m.avgFunctionLength << '\n';
    if (m.longestFunctionLines > 0) {
        out << "Longest Function : " << m.longestFunctionName  << '\n';
        out << "Length : "           << m.longestFunctionLines << " lines\n";
    } else {
        out << "Longest Function : (none)\n";
    }
    out << kSeparator << '\n';
}
```

`printSummary()` and `saveToFile()` both call `writeReport()` — there is exactly one place where report layout is defined. This means console and file output can never drift apart.

`saveToFile()` checks `file.good()` after writing, not just after opening:

```cpp
bool ReportGenerator::saveToFile(const ProjectMetrics& metrics,
                                  const std::string& outputPath) {
    std::ofstream file(outputPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return false;
    writeReport(metrics, file);
    return file.good();   // catches write-time failures (e.g., disk full)
}
```

A file can open successfully but fail partway through writing (disk full, quota exceeded, network drive disconnects). `file.good()` returns `false` if any error bit (`failbit`, `badbit`) was set during the write — not just at open time.

---

### 9.9 `main.cpp` — Pipeline Orchestrator

**File:** `src/main.cpp`  
**Responsibility:** Parse CLI arguments, drive the full pipeline for each file, and produce output.

#### `Config` Struct and `parseArgs()`

```cpp
struct Config {
    fs::path    targetPath;
    std::string outputFile;   // empty = console only
};

static std::optional<Config> parseArgs(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: cma <path> [--out <report.txt>]\n";
        return std::nullopt;
    }
    Config cfg;
    cfg.targetPath = argv[1];
    for (int i = 2; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--out") cfg.outputFile = argv[i + 1];
    }
    return cfg;
}
```

`parseArgs()` returns `std::optional<Config>`: the caller checks `if (!cfg) return 1;` rather than using exit codes or exceptions inside the function. This is a clean way to signal "parsing failed" without throwing.

The loop condition `i < argc - 1` ensures `argv[i + 1]` is always a valid access — if `--out` is the last argument with no value following it, the option is silently ignored.

#### Per-File Pipeline Loop

```cpp
for (const auto& filepath : files) {
    const auto source = FileScanner::readFile(filepath);
    if (!source) { ++skipped; continue; }

    const int lineCount =
        static_cast<int>(std::count(source->begin(), source->end(), '\n')) + 1;

    Lexer lexer(*source);
    auto  tokens = lexer.tokenize();

    Parser    parser(tokens, lineCount);
    FileMetrics fm = parser.analyze();

    engine.addFile(filepath.string(), std::move(fm));
}
```

`std::count` to count `'\n'` characters: O(n) in file size, called once per file. The `+ 1` accounts for the final line which may not have a trailing newline.

`std::move(fm)` when calling `addFile()`: after the loop body, `fm` is not used again, so moving it into the engine avoids copying the `functions` and `classes` vectors inside the struct.

---

## 10. Folder Structure Explanation

```
CodeMetricsAnalyzer/
├── CMakeLists.txt                   # Build configuration — 5 static libs + 1 executable
├── include/                         # All public headers (shared include root)
│   ├── lexer/
│   │   ├── Token.h                  # TokenType enum, Token struct, inline predicates
│   │   └── Lexer.h                  # Lexer class declaration
│   ├── filesystem/
│   │   └── FileScanner.h            # FileScanner class declaration
│   ├── parser/
│   │   ├── ParseResult.h            # FunctionInfo, ClassInfo, FileMetrics structs
│   │   └── Parser.h                 # Parser class declaration
│   ├── metrics/
│   │   ├── Metrics.h                # ProjectMetrics struct
│   │   └── MetricsEngine.h          # MetricsEngine class declaration
│   └── report/
│       └── ReportGenerator.h        # ReportGenerator class declaration
└── src/                             # All implementation files
    ├── main.cpp                     # Entry point, pipeline orchestration
    ├── lexer/
    │   └── Lexer.cpp                # Lexer implementation + keyword table
    ├── filesystem/
    │   └── FileScanner.cpp          # FileScanner implementation + extension set
    ├── parser/
    │   └── Parser.cpp               # Parser + FileMetrics derived method definitions
    ├── metrics/
    │   └── MetricsEngine.cpp        # MetricsEngine implementation
    └── report/
        └── ReportGenerator.cpp      # ReportGenerator implementation + separator constant
```

**Why this structure?**

Each `include/<module>/` directory maps directly to one CMake static library (`cma_lexer`, `cma_filesystem`, etc.). The `INCLUDE_DIR` variable in `CMakeLists.txt` points to the top-level `include/` directory:

```cmake
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
target_include_directories(cma_lexer PUBLIC ${INCLUDE_DIR})
```

This means any consumer of `cma_lexer` writes `#include "lexer/Token.h"` — the subdirectory is part of the include path, making it immediately clear which module owns each header. A flat `include/` with all headers at the same level would work but would lose that visual ownership.

**Build directory** is not in the tree — CMake generates it externally (`cmake -B build && cmake --build build`).

---

## 11. Data Flow

```
argv[1], argv[2..n]
       │
       ▼
   parseArgs()
       │ Config{targetPath, outputFile}
       ▼
   FileScanner::scan()
       │ vector<filesystem::path>   (sorted, filtered)
       ▼ (for each path)
   FileScanner::readFile()
       │ optional<string>           (raw source text)
       ▼
   std::count('\n') + 1
       │ int lineCount
       ▼
   Lexer::tokenize()
       │ vector<Token>              (20 types, line+col tagged)
       ▼
   Parser::classifyLines()          (Pass 1)
       │ vector<LineType>           (BLANK / COMMENT / CODE, per line)
       ▼
   Parser::walkTokens()             (Pass 2)
       │ in-place state: braceDepth, inFunction, cyclomaticComplexity, …
       ▼
   Parser::analyze() returns
       │ FileMetrics                (all per-file metrics, functions vector, classes vector)
       ▼
   MetricsEngine::addFile()
       │ stored in vector<pair<string, FileMetrics>>
       ▼ (after all files)
   MetricsEngine::compute()
       │ ProjectMetrics             (summed + max-reduced aggregates)
       ▼
   ReportGenerator::writeReport()
       │ → stdout
       │ → optional file
       ▼
   Program exits 0
```

All data at every stage is a concrete value type. There are no shared pointers, no global state, and no communication between modules except through function return values and parameters.

---

## 12. Algorithms Used

### 12.1 Character-Level State Machine (Lexer)

The Lexer uses a greedy, priority-ordered dispatch: at each position, it examines the current character (and one lookahead) to decide which sub-lexer to invoke. Each sub-lexer consumes as much input as belongs to its token type and returns. This is equivalent to a deterministic finite automaton (DFA) implemented manually rather than as a table.

**Time complexity:** O(n) in source file length — each character is consumed exactly once by `advance()`.  
**Space complexity:** O(t) where t is the number of tokens produced.

### 12.2 Two-Pass Line Classification (Parser Pass 1)

Each token in the stream is visited once. For each token, its associated line(s) are marked in the `m_lineTypes` array using the upgrade-only rule (BLANK → COMMENT → CODE, never backward). Block comments iterate their value string to find newlines and mark each spanned line.

**Time complexity:** O(n) in token count + O(c) for block comment character scanning (c = total comment text length).  
**Space complexity:** O(L) where L = total source lines (for the `m_lineTypes` vector).

### 12.3 Single-Pass Structural Analysis (Parser Pass 2)

One linear scan over the token vector. All state (brace depth, current function tracking, cyclomatic complexity accumulator) is updated in-place as tokens are visited. No backtracking.

**Time complexity:** O(n) in token count. `skipTrailingSpecifiers()` does lookahead within Pass 2 (scanning forward from `)` to `{`), but each token is visited at most twice — once in the main loop and once in `skipTrailingSpecifiers` — so the overall pass remains O(n).

`findMatchingParen()` also does lookahead (scanning forward from `(` to `)`) with balanced depth tracking. In the worst case (a function signature with deeply nested default arguments), this is O(n); in practice, function signatures are short.

### 12.4 Paren Matching (Balanced Depth Counter)

```
depth = 1
for each token starting after OPEN_PAREN:
    if OPEN_PAREN:  depth++
    if CLOSE_PAREN: depth--;  if depth == 0: return this index
```

Classic O(n) balanced-parentheses scan. Returns `m_tokens.size()` (a sentinel out-of-range value) on failure rather than throwing.

### 12.5 Substring Search for TODO/FIXME (std::string::find)

Two separate linear scans over each comment token's value using `std::string::find`. The inner loop advances `pos` by the keyword length after each match to avoid counting overlapping occurrences (though `TODOTODO` is unlikely in practice).

**Time complexity:** O(c * k) where c = comment text length, k = keyword length (4 or 5). Dominated by O(c).

### 12.6 Longest Function (std::max_element with Lambda)

```cpp
return &*std::max_element(
    functions.begin(), functions.end(),
    [](const FunctionInfo& a, const FunctionInfo& b) {
        return a.lineCount() < b.lineCount();
    });
```

`std::max_element` is O(n) in the number of functions. The lambda comparator is called O(n) times. Returns an iterator which is dereferenced to get a reference, then its address is taken to return a pointer into the vector.

### 12.7 Lexicographic File Sort (std::sort)

`std::sort(files.begin(), files.end())` on `std::vector<filesystem::path>` uses the default `operator<` for `filesystem::path`, which is lexicographic on the path string. This is O(f log f) where f = number of files found.

### 12.8 File Reading (rdbuf Bulk Transfer)

`buf << stream.rdbuf()` is a single `std::streambuf::sgetn`-equivalent transfer from the file stream's internal buffer into the `ostringstream`. This avoids the loop-based line-by-line or chunk-by-chunk alternatives. The OS may satisfy this with a single `read()` syscall for files that fit in the kernel's page cache.

---

## 13. Data Structures Used

| Structure | Location | Purpose | Why Chosen |
|-----------|----------|---------|------------|
| `std::vector<Token>` | Lexer output | Ordered token stream | Random-access needed by Parser for lookahead; tokens always produced left-to-right |
| `std::unordered_set<std::string>` | `kKeywords` (Lexer.cpp) | O(1) keyword membership test | Called once per identifier token; hash lookup faster than sorted binary search at < 100 elements |
| `std::unordered_set<std::string>` | `kCppExtensions` (FileScanner.cpp) | O(1) extension membership test | Only 8 elements; `count()` membership test is the only operation |
| `std::unordered_set<std::string>` | `kPrimitiveTypes` (Parser.cpp) | O(1) primitive type membership test | Called for every IDENTIFIER token; static local so initialized once |
| `std::unordered_set<std::string>` | `kSpecifiers` (Parser.cpp) | O(1) specifier keyword test | Called in `skipTrailingSpecifiers()` for each token after `)` |
| `std::vector<LineType>` | `m_lineTypes` (Parser) | Per-line classification map | Index = line number − 1; O(1) access by line; pre-allocated to `totalLines` |
| `std::vector<FunctionInfo>` | `FileMetrics::functions` | Per-file function list | Ordered; supports `max_element`, iteration; grows dynamically as functions are detected |
| `std::vector<ClassInfo>` | `FileMetrics::classes` | Per-file class/struct/enum/namespace list | Same rationale as functions |
| `std::vector<std::pair<std::string, FileMetrics>>` | `MetricsEngine::m_files` | Per-file metrics store | Insertion order preserved; sequential scan in `compute()` |
| `std::optional<std::string>` | `readFile()` return | Nullable file content | Expresses "file content or absence of content" without exceptions or null pointers |
| `std::optional<Config>` | `parseArgs()` return | Nullable parsed arguments | Expresses "valid args or parse failure" cleanly |
| `std::filesystem::path` | `FileScanner`, `main.cpp` | Typed file path | OS-correct path handling; `extension()`, comparison, and streaming built in |
| `std::ostringstream` | `readFile()` | Accumulation buffer for file content | Lets `rdbuf()` stream directly into an in-memory string buffer |

---

## 14. Design Patterns Used

### 14.1 Pipeline Pattern

The core architecture is a linear pipeline where each stage transforms its input into an output type consumed by the next stage exclusively through return values. No stage has back-references to earlier stages; data flows in one direction only.

```
FileScanner → Lexer → Parser → MetricsEngine → ReportGenerator
```

This pattern makes each stage independently testable and independently replaceable — swapping in a different Lexer implementation requires changing only the `main.cpp` construction site.

### 14.2 Static Utility Class (ReportGenerator)

`ReportGenerator` has no instance state and all methods are `static`. In C++ this is a common pattern for grouping related free functions under a namespace-like scope that still benefits from access control (`private writeReport()`).

### 14.3 Upgrade-Only State Machine (Line Classification)

The `classifyLines()` pass implements a simple state machine where each line's state can only transition in the direction BLANK → COMMENT → CODE, never backward. This is the defining invariant that makes the priority rule ("a line with both code and a comment is CODE") correct and simple.

### 14.4 Anonymous Namespace for Translation-Unit-Private Constants

Both `FileScanner.cpp` and `ReportGenerator.cpp` use anonymous namespaces to define constants (`kCppExtensions`, `kSeparator`) with internal linkage. This is the modern C++ alternative to `static` for non-member symbols — it achieves the same linkage effect while also allowing type definitions and using-declarations.

### 14.5 Sentinel Return Value (findMatchingParen, skipTrailingSpecifiers)

Both `findMatchingParen()` and `skipTrailingSpecifiers()` return an index into `m_tokens`. When the search fails, they return `m_tokens.size()` — a value that is always out of range. The callers check `if (result >= m_tokens.size()) return;`. This avoids `std::optional` overhead for a case where the index is always used immediately and the "not found" state triggers a simple early return. The pattern is similar to `std::string::npos`.

### 14.6 Two-Pass Analysis

Separating line classification (Pass 1) from structural analysis (Pass 2) is a deliberate design choice. Pass 1 needs to handle multi-line block comments (spanning multiple lines) to produce correct `blankLines`/`commentLines` counts. Pass 2 needs to track brace depth and function boundaries. Combining both passes into one would require complex state management for the comment-line spanning case.

---

## 15. C++17 Concepts Used

### `std::filesystem` (C++17)

Used throughout `FileScanner`. Key APIs:
- `std::filesystem::path` — typed path object; `extension()`, `string()`, and comparison operators
- `std::filesystem::exists(path, error_code)` — non-throwing existence check
- `std::filesystem::is_regular_file(path)` — filters out symlinks, pipes, sockets
- `std::filesystem::is_directory(path)` — directory detection
- `std::filesystem::recursive_directory_iterator` with `directory_options::skip_permission_denied`
- `std::filesystem::filesystem_error` — typed exception from filesystem operations

### `std::optional<T>` (C++17)

Used for two nullable return values:
- `FileScanner::readFile()` → `std::optional<std::string>` — file content or absence
- `parseArgs()` → `std::optional<Config>` — parsed config or argument error

`std::nullopt` is returned on failure; the caller dereferences with `*source` or `cfg->targetPath` after checking.

### Structured Bindings (C++17)

```cpp
for (const auto& [filename, fm] : m_files) { ... }
```

Unpacks `std::pair<std::string, FileMetrics>` into named variables. Used in `MetricsEngine::compute()`.

### `[[nodiscard]]` Attribute (C++17)

Applied to all functions whose return value must be checked:
- `Lexer::tokenize()` — not checking means throwing away the token stream
- `FileScanner::scan()`, `FileScanner::readFile()` — not checking means ignoring I/O results
- `FileScanner::isCppFile()` — not checking the filter result would silently include all files
- `Parser::analyze()` — not checking discards all metrics
- `MetricsEngine::compute()` — not checking discards the project report
- `ReportGenerator::saveToFile()` — not checking ignores write errors

### `std::move` and Move Semantics (C++11/14/17)

- `FileScanner` constructor: `m_rootPath(std::move(rootPath))` — avoids copying a path string
- `engine.addFile(filepath.string(), std::move(fm))` — moves `FileMetrics` (including its `vector<FunctionInfo>` and `vector<ClassInfo>`) without copying
- `MetricsEngine::addFile(...)`: `m_files.emplace_back(filename, std::move(metrics))` — in-place construction with move

### `noexcept` Specifier

Applied to all pure computational methods that have no error paths:
- `Lexer::cur()`, `advance()`, `atEnd()`, `peekAt()`, `isKeyword()`
- `FileMetrics::functionCount()`, `classCount()`, `longestFunction()`, `avgFunctionLength()`
- `FunctionInfo::lineCount()`
- `MetricsEngine::files()`

### `std::error_code` Overloads

`std::filesystem::exists(path, ec)` uses the non-throwing overload. When `ec` is set, the caller inspects the error without catching an exception — appropriate for expected error conditions (path doesn't exist) as opposed to truly exceptional ones.

### `constexpr`

`constexpr char kSeparator[]` in `ReportGenerator.cpp` — compile-time constant string, zero runtime initialization cost.

### Range-Based `for` with `const auto&`

Used consistently to iterate over `vector<Token>`, `vector<LineType>`, `vector<FunctionInfo>`, and `vector<std::pair<...>>`. The `const auto&` avoids copies while preventing accidental modification.

### `std::fixed` and `std::setprecision` (`<iomanip>`)

Used in `ReportGenerator::writeReport()` to format `avgFunctionLength` as a floating-point number with exactly one decimal place:
```cpp
out << std::fixed << std::setprecision(1) << m.avgFunctionLength;
```

### `static_cast` for Safe Numeric Conversions

Used consistently when converting between `int`, `std::size_t`, and `double` to make intent clear and suppress compiler warnings:
- `static_cast<std::size_t>(totalLines)` — avoids signed/unsigned comparison warning
- `static_cast<int>(m_files.size())` — explicit narrowing from `size_t` to `int`
- `static_cast<double>(totalFnLength) / totalFnCount` — avoids integer division

### `explicit` Constructor

`Lexer`, `FileScanner`, and `Parser` all declare their constructors `explicit`. This prevents implicit single-argument construction — e.g., `Lexer lex = someString;` would be a compile error. This is especially important for `FileScanner` where `explicit FileScanner(filesystem::path)` prevents accidental implicit path conversion from a string.

---

## 16. Input and Output Flow

### Input

```
./cma <path> [--out <output_file>]

Arguments:
  <path>           required   Path to a directory or a single .cpp/.h file
  --out <file>     optional   If present, save report to this file (overwrites)

Examples:
  ./cma /home/user/myproject
  ./cma /home/user/myproject --out report.txt
  ./cma /home/user/myproject/main.cpp
```

### Validation

- If `argc < 2`: prints usage to `stderr`, returns exit code 1
- If the path does not exist: `FileScanner::scan()` prints to `stderr`, returns empty vector, `main.cpp` prints error and returns exit code 1
- If no C++ files are found: `main.cpp` prints to `stderr` and returns exit code 1
- If a file cannot be read: it is skipped with a `stderr` warning; the run continues with remaining files

### Output (Console)

```
==================================
CODE METRICS REPORT
==================================
Files Analyzed : 14
Total Lines : 2850
Blank Lines : 510
Comments : 340
Functions : 65
Classes : 12
Variables : 280
Loops : 51
Conditions : 76
Maximum Nesting : 5
Cyclomatic Complexity : 137
Average Function Length : 19.0
Longest Function : processOrder()
Length : 78 lines
TODO Comments : 9
==================================
```

"Longest Function : (none)" is printed when no functions were detected. All metrics print even if zero, except the longest function section.

### Output (File)

When `--out` is provided, `ReportGenerator::saveToFile()` writes the identical content to the specified file path, overwriting any existing file (`std::ios::trunc`). If the write fails, a warning is printed to `stderr` but the program still exits 0 (the console output was already produced successfully).

### Exit Codes

- `0` — successful analysis (even if some files were skipped)
- `1` — argument error, no C++ files found, or path does not exist

---

## 17. Error Handling Strategy

The project uses a **no-exception public API** strategy: every function that can fail returns a value-based error signal (`std::optional`, empty vector, `bool`, or `std::error_code`) rather than throwing. Exceptions are only caught (not thrown) where the standard library may throw them.

### Layer-by-Layer

**CLI Layer (main.cpp)**
- Invalid argument count → `parseArgs()` returns `std::nullopt` → `main` returns `1`
- No files found → early exit with `stderr` message

**FileScanner**
- Non-existent path → `std::error_code` overload of `exists()`, no exception, returns empty vector with `stderr` message
- Path is not a file or directory → `stderr` message, returns empty vector
- Permission-denied subdirectory → `directory_options::skip_permission_denied` silently skips
- Other filesystem error during traversal → `catch (filesystem_error&)` returns partial results
- File open failure → `std::nullopt` from `readFile()`
- File read I/O error → checks `stream.bad()` (not `fail()`), returns `std::nullopt`

**Lexer**
- Unterminated string literal → breaks on `'\n'`, emits partial `STRING_LITERAL` token, continues
- Unclosed block comment → emits whatever was consumed when `atEnd()` is reached, no exception
- Unrecognized character → emits `PUNCTUATION` token, continues

**Parser**
- Token line number out of bounds → `mark()` lambda silently ignores the line
- Unmatched parenthesis → `findMatchingParen()` returns `m_tokens.size()` sentinel, `tryBeginFunction()` returns early
- Missing function body brace → `skipTrailingSpecifiers()` finds a non-`{` token, `tryBeginFunction()` returns early

**MetricsEngine**
- Empty file list → `compute()` returns a zero-initialized `ProjectMetrics` (handled correctly by ReportGenerator)

**ReportGenerator**
- File open failure → returns `false`, caller prints `stderr` warning
- Write failure mid-report → `file.good()` returns `false` after `writeReport()`, function returns `false`

**What is never done:**
- No `catch(...)` that swallows exceptions silently
- No `abort()` or `exit()` inside modules (only in `main`)
- No raw pointer dereference that could be null without a prior check

---

## 18. Performance Analysis

### Per-File Pipeline Cost

For a source file with S characters producing T tokens and L lines:

| Stage | Time | Space | Notes |
|-------|------|-------|-------|
| `readFile()` | O(S) | O(S) | Single `rdbuf()` transfer |
| Line count (`std::count`) | O(S) | O(1) | Single pass over the string |
| `Lexer::tokenize()` | O(S) | O(T) | Each char consumed once; reserve avoids reallocs |
| `Parser::classifyLines()` | O(T + C) | O(L) | T = token count, C = comment char count |
| `Parser::walkTokens()` | O(T) | O(F + K) | F = functions, K = classes found |
| `MetricsEngine::addFile()` | O(1) amortized | O(1) | `emplace_back` + move |

### Project-Level Aggregation Cost

| Stage | Time | Space |
|-------|------|-------|
| `MetricsEngine::compute()` | O(N × F_avg) | O(1) extra | N = files, F_avg = avg functions per file |
| `std::sort(files)` | O(N log N) | O(log N) | Lexicographic path sort |

### Practical Notes

- The `unordered_set` keyword lookup in the Lexer fires for every identifier token. For a file with many unique identifiers, this is O(T) hash operations. The alternative (sorted vector + binary search) would be O(T log K) where K = 78 keywords — for small K, the unordered_set wins for large T.
- `tokens.reserve(source.size() / 4)` reduces reallocations but may over-allocate. For files with dense comments or string literals (which produce few tokens relative to character count), the actual token count may be well below `source.size() / 4`. This is an acceptable tradeoff.
- The inner loop in `classifyLines()` over block comment text is the only place the raw source text (as captured in token values) is re-scanned. For projects with large `/* ... */` comment blocks, this adds O(C) overhead where C is total comment character count.
- No parallel processing is implemented. Files are processed sequentially. For a 500-file project, this is acceptable; for very large codebases (5,000+ files), parallelization would be the most impactful optimization.

---

## 19. Scalability Analysis

### What Scales Well

- **Memory:** The pipeline processes one file at a time. Only the tokens for the current file, its `FileMetrics`, and the accumulated `m_files` vector are in memory simultaneously. For a 10,000-line file, the token vector is roughly 10,000 × sizeof(Token) = 10,000 × ~48 bytes ≈ 480 KB — well within modern memory limits.
- **File count:** Adding more files increases `m_files` size linearly. `compute()` is O(N × F_avg) which scales well up to tens of thousands of files.
- **Extension filtering:** The `unordered_set` lookup for extensions is O(1) regardless of how many extensions are recognized.
- **File sorting:** `std::sort` is O(N log N); for 10,000 files this is roughly 130,000 comparisons — negligible.

### Current Bottlenecks for Very Large Codebases

- **Single-threaded processing:** All files are processed sequentially. On an 8-core machine, this leaves 7 cores idle. The pipeline design is inherently parallelizable: each file's Lexer → Parser chain is independent. Adding `std::thread` or `std::for_each(std::execution::par, ...)` (C++17 parallel STL, where available) is the most impactful future improvement.
- **Full token vector in memory:** Each file's complete token stream is held in memory during parsing. For extremely large generated files (e.g., a 500 KB auto-generated .cpp), this is tens of thousands of tokens. A streaming parser that reads tokens one at a time would reduce peak memory at the cost of more complex Parser implementation.
- **No caching:** Each run re-reads and re-analyzes every file. For incremental analysis (re-checking only changed files), a file modification timestamp cache would allow unchanged files to be skipped.
- **`m_files` stores full FileMetrics:** The stored `vector<FunctionInfo>` and `vector<ClassInfo>` for every file remain in memory for the lifetime of the `MetricsEngine`. For projects with thousands of functions, this is the dominant memory consumer. If per-file detail is not needed after aggregation, those vectors could be cleared after `compute()`.

---

## 20. Limitations

These limitations are inherent to the current implementation. They are documented here honestly for portfolio and interview purposes — not as bugs, but as deliberate scope decisions appropriate for an intern/SDE-1 project.

**Variable Counting is Approximate**

Only primitive-type declarations are counted: `int`, `long`, `short`, `char`, `bool`, `float`, `double`, `unsigned`, `signed`, `auto`, `wchar_t`, `size_t`, `void`. Declarations like `std::string s`, `vector<int> v`, or `MyClass obj` are not counted. This is documented in `ParseResult.h` and `Parser.h`.

**Lambdas Are Not Counted as Functions**

The function detection heuristic requires `IDENTIFIER ( ... ) [specifiers] {` at brace depth 0 or 1. Lambda expressions have the form `[captures](...){...}` — the token immediately before `(` is a `]` (CLOSE_BRACKET), not an IDENTIFIER. So lambdas do not match the heuristic and are not included in the function count. Documented in `Parser.h`.

**Nested Functions and Local Classes Are Not Tracked**

A function defined inside another function body (rare in C++ but legal via local classes) would have `m_braceDepth > 1` at its definition point, causing `tryBeginFunction()` to return early. Local class methods are similarly excluded. Documented in `Parser.h`.

**Templates with Complex Return Types May Be Missed**

A function template like `template<typename T> std::enable_if_t<...> foo(T x) { ... }` has a keyword token (`template`) before the return type before the identifier. The heuristic identifies the function by the `IDENTIFIER (` sequence, but if the return type is complex enough that the relevant `IDENTIFIER` appears at a position where brace depth tracking has already moved on, detection may fail.

**Cyclomatic Complexity Is Summed Across Files**

Each file's `cyclomaticComplexity` starts at 1 (base path). A project with N files reported as zero complexity would show N in the total, not 0. This matches the intent (sum of all file complexities) but means the number is not directly comparable to a single-module McCabe complexity.

**`ClassInfo::Kind` Is Not Exposed in Report**

The `ClassInfo` struct distinguishes between `CLASS`, `STRUCT`, `ENUM`, and `NAMESPACE`, but the final report counts them all together as "Classes". The breakdown by kind is available in the data but is not shown.

**No Fan-In / Fan-Out Coupling Metrics**

The project instructions mentioned coupling metrics (fan-in, fan-out). The implementation tracks `#include` directive count (`includeCount`) but does not compute which files include which other files, or how many times a function is called from external modules. This would require building an include dependency graph — a significant additional subsystem not implemented in this version.

**Single-Threaded Only**

No parallelism. On large codebases, this is the most significant performance limitation.

**C++ Only**

The extension filter and keyword set are C++-specific. The tool cannot analyze C, Java, Python, or any other language without adding new extension sets and rewriting the keyword table.

**No Column-Level Error Reporting**

Column numbers (`Token::col`) are tracked in the Lexer but are never used in the final output. They exist for potential future IDE-integration or precise diagnostic output.

---

## 21. Future Enhancements

These are concrete, well-defined extensions that build naturally on the current architecture.

**Unit Test Suite (Phase 4 — Planned)**
Each module is a separate CMake static library, enabling test executables that link only the module under test. Google Test or Catch2 would provide the framework. Target test cases: Lexer edge cases (unterminated literals, block comments spanning 50 lines, backslash-continuation directives), Parser count verification against known-correct files, MetricsEngine aggregation correctness.

**Per-File Detailed Report**
`MetricsEngine::m_files` already stores per-file data. Adding a `--verbose` flag and a per-file section to `ReportGenerator::writeReport()` requires no changes to the analysis pipeline — only the output stage.

**ClassInfo Kind Breakdown**
`ClassInfo::Kind` already distinguishes `CLASS`, `STRUCT`, `ENUM`, `NAMESPACE`. The report could show these separately with one additional loop in `ReportGenerator::writeReport()`.

**Parallel File Processing**
The per-file pipeline (read → lex → parse) is fully independent between files. Wrapping the loop in `main.cpp` with `std::for_each(std::execution::par_unseq, ...)` (C++17 parallel STL, compiler and platform dependent) or a thread pool would reduce wall-clock time on multi-core machines proportionally.

**Include Dependency Graph**
`FileMetrics::includeCount` and `Parser::classifyLines()` already detect `#include` directives. Storing the full directive text (extracting the filename from `#include "foo.h"` or `#include <bar>`) and building an include graph in the MetricsEngine would enable fan-in and fan-out coupling metrics.

**Incremental Analysis**
Cache `FileMetrics` keyed by `(filepath, last_modified_time)`. On subsequent runs, re-analyze only files whose modification time has changed. This would make repeat runs on large codebases nearly instant.

**JSON Output Mode**
`ProjectMetrics` is a simple struct — serializing it to JSON (using a single-header library like nlohmann/json or a hand-written serializer) would allow integration with dashboards, CI pipelines, or spreadsheet tools.

**Function-Level Complexity**
The Parser already knows function start/end lines and maintains a cyclomatic complexity counter during `walkTokens()`. Adding a per-function complexity accumulator (reset at function start) would enable identifying the most complex functions, not just the longest.

---

## 22. Real-World Use Cases

**Code Review Preparation**
Before submitting a pull request, a developer can run CMA on the changed files to check if any function has grown beyond a team's agreed threshold (e.g., "no function longer than 50 lines") or if cyclomatic complexity has increased beyond a limit (e.g., "no file above 30").

**Technical Debt Tracking**
Running CMA on each commit in a CI pipeline and logging the TODO/FIXME count, cyclomatic complexity, and average function length over time produces a trend chart that shows whether a codebase is getting cleaner or more complex.

**Onboarding New Team Members**
A team lead can point a new hire at CMA to get a quick orientation metric dump: how many files, functions, and classes the project has; which functions are the longest and most complex; and how much of the code is commented.

**Educational Settings**
Instructors grading student C++ assignments can use CMA to quickly assess whether a submission has reasonable function lengths, adequate commenting, and no runaway complexity — without reading the code line by line.

**Legacy Codebase Assessment**
When inheriting a legacy C++ codebase, CMA provides an immediate snapshot: total lines, function count, longest and most complex functions, and outstanding TODO markers — in seconds, without a build environment.

---

## 23. Technical Skills Demonstrated

This project provides concrete evidence of the following skills during an intern/SDE-1 interview:

### C++17 Language Skills

- `std::filesystem` API: directory traversal, path manipulation, error handling
- `std::optional<T>`: nullable return without exceptions, `nullopt` patterns
- Structured bindings: `const auto& [k, v]` unpacking of `std::pair`
- Move semantics: `std::move` for parameter passing and `emplace_back`, value-category awareness
- `[[nodiscard]]`, `noexcept`, `explicit`: attribute and specifier usage for API correctness
- `constexpr` and anonymous namespace: compile-time constants with correct linkage
- `static_cast`: safe numeric conversions, signed/unsigned handling
- `std::unordered_set`: hash-based membership testing, O(1) lookup
- Range-based `for` with `const auto&`
- Enum class: scoped, strongly-typed enumerations

### Data Structures

- `std::vector`: dynamic array, reserve heuristics, move-construction
- `std::unordered_set<std::string>`: keyword/extension membership
- `std::optional<T>`: nullable value representation
- `std::pair<K, V>` and structured bindings
- `std::filesystem::path`: typed path with lexicographic comparison

### Algorithms

- Character-level state machine (Lexer)
- Two-pass linear scan (Parser)
- Balanced bracket matching (paren matching)
- Substring search with `std::string::find`
- `std::max_element` with lambda comparator
- `std::sort` for deterministic ordering
- `std::count` for character frequency

### Software Design

- Modular architecture: 5 independent static libraries with clear interface boundaries
- Pipeline design pattern: each stage transforms its input to a typed output
- Single Responsibility: each module (Lexer, Parser, Scanner, Engine, Report) has exactly one job
- Separation of Concerns: I/O, analysis, aggregation, and formatting are distinct stages
- Static utility class (ReportGenerator): all-static design with private implementation function
- Anonymous namespace: translation-unit-private constants without global symbol pollution
- Upgrade-only state machine: one-directional state transitions for line classification
- Sentinel return values: `m_tokens.size()` as a safe "not found" indicator

### Build System

- CMake: `add_library(STATIC)`, `target_include_directories`, `target_link_libraries` with transitive dependency chain
- Per-module static libraries for compiler caching and targeted test linking
- Compiler-specific guards (GCC < 9 `stdc++fs`)
- Strict warning flags: `-Wall -Wextra -Wpedantic -Werror`
- `CMAKE_CXX_EXTENSIONS OFF` for standards-conforming builds

### Error Handling

- `std::error_code` overloads for non-throwing filesystem operations
- `std::optional` for expected-absence scenarios
- Graceful lexer recovery (no throw on malformed input)
- Partial results on traversal errors (not all-or-nothing)
- Distinction between `stream.bad()` and `stream.fail()` in file reading

### Interview Talking Points

1. **"Why five static libraries instead of one?"** — Compiler caches each library independently; a change to `Parser.cpp` does not recompile `Lexer.cpp`. Tests can link only the module they need.

2. **"Why `std::optional` for `readFile()`?"** — It explicitly encodes the expectation that a file may be unreadable in the return type. An exception would be appropriate for unexpected failures; `optional` is correct for expected failure cases like permission errors.

3. **"Why emit NEWLINE as a token?"** — It allows the Parser to reconstruct per-line information from a single linear scan of the token vector, without storing a separate line-number-to-token-range map.

4. **"How does the function detection heuristic work?"** — It looks for `IDENTIFIER ( ... ) [specifiers] {` at brace depth 0 or 1. This covers member functions (depth 1, inside class body) and free functions (depth 0). Lambdas don't match because they start with `[`, not an identifier.

5. **"What are the known limitations?"** — Variable counting is approximate (primitives only), lambdas are excluded, nested functions are not tracked. These are documented scope decisions, not oversights.

6. **"Why `sort` the file list?"** — `recursive_directory_iterator` order is implementation-defined and may differ between Linux (ext4), macOS (HFS+), and Windows (NTFS). Sorting guarantees the same report output for the same input regardless of OS.

---

*This documentation was generated in Phase 6 of the Code Metrics Analyzer project, based strictly on the implemented source code. No features are described that are not present in the actual implementation.*
