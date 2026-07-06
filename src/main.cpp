#include "filesystem/FileScanner.h"
#include "lexer/Lexer.h"
#include "metrics/MetricsEngine.h"
#include "parser/Parser.h"
#include "report/ReportGenerator.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

namespace fs = std::filesystem;
using namespace cma;

// ── CLI argument parsing ───────────────────────────────────────────────────
struct Config {
    fs::path    targetPath;
    std::string outputFile;  // empty = console only
};

static std::optional<Config> parseArgs(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: cma <path> [--out <report.txt>]\n";
        return std::nullopt;
    }

    Config cfg;
    cfg.targetPath = argv[1];

    for (int i = 2; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--out") {
            cfg.outputFile = argv[i + 1];
        }
    }
    return cfg;
}

// ── Pipeline ───────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    const auto cfg = parseArgs(argc, argv);
    if (!cfg) return 1;

    // Collect source files — FileScanner handles existence checks, the
    // single-file-vs-directory distinction, and permission errors internally.
    const FileScanner scanner(cfg->targetPath);
    const std::vector<fs::path> files = scanner.scan();

    if (files.empty()) {
        std::cerr << "No C++ source files found in: " << cfg->targetPath << '\n';
        return 1;
    }

    std::cout << "Analyzing " << files.size() << " file(s)...\n";

    // ── Core pipeline: FileScanner → Lexer → Parser → MetricsEngine ────────
    MetricsEngine engine;
    int skipped = 0;

    for (const auto& filepath : files) {
        const auto source = FileScanner::readFile(filepath);
        if (!source) {
            std::cerr << "  [skip] cannot read: " << filepath << '\n';
            ++skipped;
            continue;
        }

        // Newline count + 1: counts the final line even without a
        // trailing '\n'. An empty file yields 1 (one blank line).
        const int lineCount =
            static_cast<int>(std::count(source->begin(), source->end(), '\n')) + 1;

        Lexer lexer(*source);
        auto  tokens = lexer.tokenize();

        Parser    parser(tokens, lineCount);
        FileMetrics fm = parser.analyze();

        engine.addFile(filepath.string(), std::move(fm));
    }

    if (skipped > 0)
        std::cout << "Warning: skipped " << skipped << " unreadable file(s).\n";

    // ── Report ─────────────────────────────────────────────────────────────
    const ProjectMetrics report = engine.compute();
    ReportGenerator::printSummary(report, std::cout);

    if (!cfg->outputFile.empty()) {
        if (ReportGenerator::saveToFile(report, cfg->outputFile))
            std::cout << "Report saved to: " << cfg->outputFile << '\n';
        else
            std::cerr << "Warning: could not write to: " << cfg->outputFile << '\n';
    }

    return 0;
}
