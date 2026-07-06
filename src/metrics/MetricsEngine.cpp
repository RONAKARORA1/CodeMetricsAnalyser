#include "metrics/MetricsEngine.h"

#include <algorithm>

namespace cma {

void MetricsEngine::addFile(const std::string& filename, FileMetrics metrics) {
    m_files.emplace_back(filename, std::move(metrics));
}

const std::vector<std::pair<std::string, FileMetrics>>&
MetricsEngine::files() const noexcept {
    return m_files;
}

ProjectMetrics MetricsEngine::compute() const {
    ProjectMetrics pm;
    pm.filesAnalyzed = static_cast<int>(m_files.size());

    long totalFnLength = 0;
    int  totalFnCount  = 0;

    for (const auto& [filename, fm] : m_files) {
        pm.totalLines        += fm.totalLines;
        pm.blankLines        += fm.blankLines;
        pm.commentLines      += fm.commentLines;
        pm.codeLines         += fm.codeLines;
        pm.functionCount     += fm.functionCount();
        pm.classCount        += fm.classCount();
        pm.variableCount     += fm.variableCount;
        pm.includeCount      += fm.includeCount;
        pm.loopCount         += fm.loopCount;
        pm.conditionCount    += fm.conditionCount;
        pm.tryCatchCount     += fm.tryCatchCount;
        pm.cyclomaticComplexity += fm.cyclomaticComplexity;
        pm.todoCount         += fm.todoCount;
        pm.maxNestingDepth    = std::max(pm.maxNestingDepth, fm.maxNestingDepth);

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
        ? static_cast<double>(totalFnLength) / totalFnCount
        : 0.0;

    return pm;
}

} // namespace cma
