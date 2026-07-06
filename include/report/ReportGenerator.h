#pragma once

#include "metrics/Metrics.h"

#include <ostream>
#include <string>

namespace cma {

// Formats a ProjectMetrics summary for human consumption.
//
// Single responsibility: text formatting and output. ReportGenerator never
// computes anything — it only renders numbers MetricsEngine already produced.
//
// Usage:
//   ReportGenerator::printSummary(report, std::cout);
//   if (!ReportGenerator::saveToFile(report, "report.txt")) { ... }
class ReportGenerator {
public:
    // Writes the formatted report to any output stream (console, etc.)
    static void printSummary(const ProjectMetrics& metrics, std::ostream& out);

    // Writes the same report to a named file, overwriting any existing
    // content. Returns false if the file couldn't be opened or a write
    // error occurred partway through (e.g. disk full).
    [[nodiscard]] static bool saveToFile(const ProjectMetrics& metrics,
                                          const std::string& outputPath);

private:
    // Shared by both public entry points — the single source of truth for
    // report layout, so console and file output can never drift apart.
    static void writeReport(const ProjectMetrics& metrics, std::ostream& out);
};

} // namespace cma
