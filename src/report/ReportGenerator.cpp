#include "report/ReportGenerator.h"

#include <fstream>
#include <iomanip>

namespace cma {

namespace {
constexpr char kSeparator[] = "==================================";
} // anonymous namespace

void ReportGenerator::printSummary(const ProjectMetrics& metrics, std::ostream& out) {
    writeReport(metrics, out);
}

bool ReportGenerator::saveToFile(const ProjectMetrics& metrics,
                                  const std::string& outputPath) {
    std::ofstream file(outputPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    writeReport(metrics, file);
    return file.good(); // catches write-time failures, not just open failures
}

void ReportGenerator::writeReport(const ProjectMetrics& m, std::ostream& out) {
    out << kSeparator << '\n';
    out << "CODE METRICS REPORT\n";
    out << kSeparator << '\n';

    out << "Files Analyzed : "        << m.filesAnalyzed        << '\n';
    out << "Total Lines : "           << m.totalLines           << '\n';
    out << "Blank Lines : "           << m.blankLines           << '\n';
    out << "Comments : "              << m.commentLines         << '\n';
    out << "Functions : "             << m.functionCount        << '\n';
    out << "Classes : "               << m.classCount           << '\n';
    out << "Variables : "             << m.variableCount        << '\n';
    out << "Loops : "                 << m.loopCount            << '\n';
    out << "Conditions : "            << m.conditionCount       << '\n';
    out << "Maximum Nesting : "       << m.maxNestingDepth      << '\n';
    out << "Cyclomatic Complexity : " << m.cyclomaticComplexity << '\n';

    out << "Average Function Length : "
        << std::fixed << std::setprecision(1) << m.avgFunctionLength << '\n';

    if (m.longestFunctionLines > 0) {
        out << "Longest Function : " << m.longestFunctionName    << '\n';
        out << "Length : "           << m.longestFunctionLines   << " lines\n";
    } else {
        out << "Longest Function : (none)\n";
    }

    out << "TODO Comments : " << m.todoCount << '\n';
    out << kSeparator << '\n';
}

} // namespace cma
