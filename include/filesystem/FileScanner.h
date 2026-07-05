#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cma {

// Discovers C++ source files within a directory tree and reads file content.
//
// Single responsibility: file I/O only.
// This class does not tokenize, parse, or analyze — those are downstream jobs.
//
// Usage:
//   FileScanner scanner("/path/to/project");
//   for (const auto& path : scanner.scan()) {
//       auto content = FileScanner::readFile(path);
//       if (content) { /* pass to Lexer */ }
//   }
class FileScanner {
public:
    // rootPath may be a directory (scanned recursively) or a single file.
    // Takes by value and moves: no copy of the path string on construction.
    explicit FileScanner(std::filesystem::path rootPath);

    // Walks the directory tree and returns all C++ source/header file paths.
    // - Sorts results for deterministic output across platforms.
    // - Silently skips permission-denied entries.
    // - Returns an empty vector (never throws) if rootPath is invalid.
    [[nodiscard]] std::vector<std::filesystem::path> scan() const;

    // Reads the entire content of filePath into a std::string.
    // Returns std::nullopt if the file cannot be opened or an I/O error occurs.
    // static: requires no instance state — pure path → content transformation.
    [[nodiscard]] static std::optional<std::string> readFile(
        const std::filesystem::path& filePath);

private:
    std::filesystem::path m_rootPath;

    // Returns true if path has a recognized C++ source or header file extension.
    [[nodiscard]] static bool isCppFile(const std::filesystem::path& path);
};

} // namespace cma
