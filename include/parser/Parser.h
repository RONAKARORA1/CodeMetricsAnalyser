#pragma once

#include "ParseResult.h"
#include "lexer/Token.h"

#include <vector>

namespace cma {

// Analyzes one file's token stream and fills a FileMetrics struct.
//
// ── Algorithm overview ────────────────────────────────────────────────────
// Pass 1 — classifyLines()
//   Assigns each source line one of: BLANK | COMMENT | CODE.
//   Block comments that span multiple lines mark all those lines COMMENT.
//   A line is CODE if it contains any token other than a comment.
//
// Pass 2 — walkTokens()
//   Single linear scan over the token stream; O(n) in token count.
//   Tracks:
//     • Brace depth  → max nesting depth
//     • Keywords     → loop/condition/exception counts, class detection
//     • Identifiers  → function-definition detection (heuristic)
//     • Operators    → && / || / ? contribute to cyclomatic complexity
//
// ── Function detection heuristic ─────────────────────────────────────────
// A sequence  IDENTIFIER ( ... ) [specifiers] {  at brace depth 0 or 1
// is treated as a function definition.
// "Specifiers" include const, noexcept, override, final, and constructor
// initializer lists (: member(val) ...).
// Lambdas ([captures](...){}) are not counted — they lack a name token
// before the '(' in the position this heuristic checks.
//
// ── Known limitations (acceptable at SDE-1 scope) ────────────────────────
// • Nested functions / local classes inside a function body are skipped.
// • Template specialisations with complex return types may be missed.
// • Variable counting only recognises primitive-type declarations.
class Parser {
public:
    // tokens     — from Lexer::tokenize()
    // totalLines — raw line count of the source (drives blank-line detection)
    explicit Parser(const std::vector<Token>& tokens, int totalLines);

    [[nodiscard]] FileMetrics analyze();

private:
    // ── Pass 1 ────────────────────────────────────────────────────────────
    void classifyLines();

    // ── Pass 2 ────────────────────────────────────────────────────────────
    void walkTokens();
    void handleKeyword(std::size_t idx);
    void tryBeginFunction(std::size_t identIdx);
    void handleVariableDecl(std::size_t identIdx);

    // Find the CLOSE_PAREN that matches OPEN_PAREN at openIdx.
    // Returns m_tokens.size() when not found (never throws).
    [[nodiscard]] std::size_t findMatchingParen(std::size_t openIdx) const;

    // Advance past const/noexcept/override/final/initializer-list tokens
    // that may appear between ')' and '{' in a function signature.
    // Returns the index of the next structurally meaningful token.
    [[nodiscard]] std::size_t skipTrailingSpecifiers(std::size_t afterCloseParen) const;

    // Count "TODO" and "FIXME" occurrences in a comment's text value
    [[nodiscard]] int countTodos(const std::string& commentText) const;

    const std::vector<Token>& m_tokens;
    int                        m_totalLines;
    FileMetrics                m_result;

    // ── Walk state ────────────────────────────────────────────────────────
    int  m_braceDepth = 0;
    int  m_maxDepth   = 0;

    // Active function being tracked
    bool         m_inFunction   = false;
    int          m_fnBraceDepth = 0;   // depth of the opening '{' of the function
    FunctionInfo m_currentFn;

    // ── Line classification map ───────────────────────────────────────────
    enum class LineType { BLANK, COMMENT, CODE };
    std::vector<LineType> m_lineTypes; // index = line number - 1
};

} // namespace cma
