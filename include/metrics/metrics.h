#pragma once

#include <string>

namespace cma {

// Project-wide aggregate of every FileMetrics produced during analysis.
// Built by MetricsEngine::compute(); consumed by ReportGenerator.
//
// Field meanings mirror FileMetrics one-for-one (see ParseResult.h) except
// where noted — most fields are simple per-file sums; a few track a single
// project-wide extreme instead (deepest nesting, longest function).
struct ProjectMetrics {
    // ── Scope ──────────────────────────────────────────────────────────────
    int filesAnalyzed = 0;

    // ── Size metrics (summed across all files) ────────────────────────────
    int totalLines   = 0;
    int blankLines   = 0;
    int commentLines = 0;
    int codeLines    = 0;

    // ── Structural metrics (summed across all files) ──────────────────────
    int functionCount = 0;
    int classCount    = 0;
    int variableCount = 0;
    int includeCount  = 0;

    // ── Complexity metrics ─────────────────────────────────────────────────
    int loopCount            = 0;
    int conditionCount       = 0;
    int tryCatchCount        = 0;
    int cyclomaticComplexity = 0;  // summed: each file's own McCabe score
    // NOT summed — summing per-file nesting depths would be meaningless.
    // This is the single deepest nesting level seen in any one file.
    int maxNestingDepth      = 0;

    // ── Comment-quality metrics ────────────────────────────────────────────
    int todoCount = 0;

    // ── Function-length extremes ───────────────────────────────────────────
    double      avgFunctionLength    = 0.0;
    int         longestFunctionLines = 0;
    std::string longestFunctionName;  // e.g. "processOrder()"
};

} // namespace cma
