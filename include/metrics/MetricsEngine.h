#pragma once

#include "metrics/Metrics.h"
#include "parser/ParseResult.h"

#include <string>
#include <utility>
#include <vector>

namespace cma {

// Accumulates per-file FileMetrics results and aggregates them into a
// single ProjectMetrics summary.
//
// Single responsibility: aggregation only. MetricsEngine does not lex,
// parse, or format — it just collects what Parser::analyze() produced
// for each file and reduces it to project-level totals.
//
// Usage:
//   MetricsEngine engine;
//   engine.addFile(path.string(), parser.analyze());
//   ProjectMetrics report = engine.compute();
class MetricsEngine {
public:
    // Stores metrics for one analyzed file. Moves metrics in — callers
    // should std::move an rvalue FileMetrics to avoid copying its
    // functions/classes vectors.
    void addFile(const std::string& filename, FileMetrics metrics);

    // Read-only access to every file collected so far, in insertion order.
    [[nodiscard]] const std::vector<std::pair<std::string, FileMetrics>>&
    files() const noexcept;

    // Reduces all collected FileMetrics into one ProjectMetrics.
    // Safe to call multiple times; does not mutate engine state.
    [[nodiscard]] ProjectMetrics compute() const;

private:
    std::vector<std::pair<std::string, FileMetrics>> m_files;
};

} // namespace cma
