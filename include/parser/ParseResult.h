#pragma once

#include <string>
#include <vector>

namespace cma {

// ── Per-function detail ────────────────────────────────────────────────────
struct FunctionInfo {
    std::string name;
    int         startLine = 0;
    int         endLine   = 0;

    [[nodiscard]] int lineCount() const noexcept {
        return (endLine >= startLine) ? (endLine - startLine + 1) : 0;
    }
};

// ── Per-class/struct/enum/namespace detail ────────────────────────────────
struct ClassInfo {
    enum class Kind { CLASS, STRUCT, ENUM, NAMESPACE };

    std::string name;
    int         line = 0;
    Kind        kind = Kind::CLASS;
};

// ── All metrics for a single source file ──────────────────────────────────
// Produced by Parser::analyze(); consumed by MetricsEngine and ReportGenerator.
struct FileMetrics {
    // ── Size metrics ──────────────────────────────────────────────────────
    int totalLines   = 0;
    int blankLines   = 0;
    int commentLines = 0;
    int codeLines    = 0;   // SLOC = total - blank - comment

    // ── Structural metrics ────────────────────────────────────────────────
    std::vector<FunctionInfo> functions;
    std::vector<ClassInfo>    classes;
    int                       includeCount  = 0;
    // Variable count is an approximation (primitive-type declarations only).
    // Intentionally conservative: std::string, vector<T>, etc. are not counted.
    // Documented limitation — acceptable at SDE-1 scope.
    int variableCount = 0;

    // ── Complexity metrics ────────────────────────────────────────────────
    int loopCount            = 0;  // for / while / do
    int conditionCount       = 0;  // if / switch
    int tryCatchCount        = 0;  // try blocks
    int maxNestingDepth      = 0;
    // McCabe cyclomatic complexity (simplified):
    //   1 (base) + if + switch + for + while + do + case + catch + && + || + ?
    int cyclomaticComplexity = 1;

    // ── Comment-quality metrics ───────────────────────────────────────────
    int todoCount = 0;   // TODO / FIXME markers

    // ── Derived helpers ───────────────────────────────────────────────────
    [[nodiscard]] int functionCount() const noexcept {
        return static_cast<int>(functions.size());
    }
    [[nodiscard]] int classCount() const noexcept {
        return static_cast<int>(classes.size());
    }

    // Pointer into this object's functions vector; nullptr if empty.
    [[nodiscard]] const FunctionInfo* longestFunction() const noexcept;
    [[nodiscard]] double              avgFunctionLength() const noexcept;
};

} // namespace cma
