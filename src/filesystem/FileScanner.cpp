#include "filesystem/FileScanner.h"

#include <algorithm>   // std::sort
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

namespace cma {

namespace {

// Anonymous namespace: file-local. kCppExtensions is invisible outside this
// translation unit — no linkage conflicts, no accidental external use.
// unordered_set: O(1) average lookup. For 8 extensions the difference vs
// a vector is negligible, but the intent (membership test) is clearer.
const std::unordered_set<std::string> kCppExtensions = {
    ".cpp", ".cc", ".cxx", ".c++",
    ".h",   ".hpp", ".hxx", ".h++"
};

} // anonymous namespace

// ─── Constructor ─────────────────────────────────────────────────────────────

FileScanner::FileScanner(std::filesystem::path rootPath)
    : m_rootPath(std::move(rootPath)) {}
// std::move: rootPath was passed by value, so we own it.
// Moving avoids an extra copy of the internal path string.

// ─── scan() ──────────────────────────────────────────────────────────────────

std::vector<std::filesystem::path> FileScanner::scan() const {
    std::vector<std::filesystem::path> files;
    std::error_code ec;

    // Check existence before touching anything else.
    // Passing ec to exists() avoids an exception on invalid paths;
    // the error is surfaced as a return value instead.
    if (!std::filesystem::exists(m_rootPath, ec) || ec) {
        std::cerr << "[FileScanner] Path does not exist: " << m_rootPath << '\n';
        return files;  // empty — caller gets a valid (empty) vector, not a throw
    }

    // ── Single-file shortcut ──────────────────────────────────────────────────
    // If the user points us at a specific file, skip directory traversal.
    if (std::filesystem::is_regular_file(m_rootPath)) {
        if (isCppFile(m_rootPath)) {
            files.push_back(m_rootPath);
        } else {
            std::cerr << "[FileScanner] File is not a recognised C++ source: "
                      << m_rootPath << '\n';
        }
        return files;
    }

    if (!std::filesystem::is_directory(m_rootPath)) {
        std::cerr << "[FileScanner] Path is neither a file nor a directory: "
                  << m_rootPath << '\n';
        return files;
    }

    // ── Recursive directory traversal ─────────────────────────────────────────
    // skip_permission_denied: the iterator silently moves past subdirectories
    // we cannot enter, rather than throwing filesystem_error on them.
    // We still catch filesystem_error for anything else (e.g. I/O hardware error).
    try {
        const auto opts = std::filesystem::directory_options::skip_permission_denied;

        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(m_rootPath, opts)) {

            // is_regular_file() filters out symlinks, sockets, pipes, etc.
            if (entry.is_regular_file() && isCppFile(entry.path())) {
                files.push_back(entry.path());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[FileScanner] Filesystem error during scan: " << e.what() << '\n';
        // Return whatever we collected before the error — partial results
        // are more useful than nothing.
    }

    // Sort for deterministic, reproducible output regardless of the OS's
    // directory iteration order (which is undefined by the standard).
    std::sort(files.begin(), files.end());

    return files;
}

// ─── readFile() ──────────────────────────────────────────────────────────────

std::optional<std::string> FileScanner::readFile(
    const std::filesystem::path& filePath) {

    // std::ios::in is the default for ifstream, but spelling it out makes
    // the intent explicit when someone reads this code.
    std::ifstream stream(filePath, std::ios::in);
    if (!stream.is_open()) {
        std::cerr << "[FileScanner] Cannot open: " << filePath << '\n';
        return std::nullopt;
    }

    // stream.rdbuf() reads the entire file in one shot through the
    // stream's internal buffer. Faster than getline() in a loop because
    // it avoids repeated string construction and bounds checking.
    std::ostringstream buf;
    buf << stream.rdbuf();

    // bad()  = non-recoverable hardware/OS I/O error (disk read failure, etc.)
    // eof()  = reached end of file — this is NORMAL after rdbuf(), not an error.
    // fail() = bad() || format mismatch — we don't check fail() here because
    //          eof() sets the fail bit on some platforms after rdbuf().
    if (stream.bad()) {
        std::cerr << "[FileScanner] I/O error reading: " << filePath << '\n';
        return std::nullopt;
    }

    return buf.str();
}

// ─── isCppFile() ─────────────────────────────────────────────────────────────

bool FileScanner::isCppFile(const std::filesystem::path& path) {
    // path.extension() returns the dot + suffix: ".cpp", ".h", etc.
    // Returns an empty string if the file has no extension.
    // count() on unordered_set is O(1) average — preferred over find() != end()
    // when you only care about presence, not the iterator.
    return kCppExtensions.count(path.extension().string()) > 0;
}

} // namespace cma
