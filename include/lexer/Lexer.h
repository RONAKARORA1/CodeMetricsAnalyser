#pragma once

#include "Token.h"

#include <string>
#include <vector>

namespace cma {

// Transforms raw C++ source text into a flat, ordered token stream.
//
// Design contract:
//   • NEWLINE tokens are emitted explicitly so the Parser can reconstruct
//     which source line each token lives on without storing a line map.
//   • String/char literals and comments are consumed atomically — the
//     parser never sees their interior characters, so keywords inside a
//     string can never be misidentified.
//   • On malformed input (unterminated string, unclosed block comment)
//     the Lexer recovers gracefully and continues rather than throwing.
//
// Usage:
//   Lexer lexer(sourceText);
//   auto tokens = lexer.tokenize();
class Lexer {
public:
    explicit Lexer(const std::string& source);

    // Returns the complete token stream, terminated with END_OF_FILE.
    [[nodiscard]] std::vector<Token> tokenize();

private:
    // ── Lexing helpers (one per token category) ────────────────────────
    Token lexIdentifierOrKeyword();
    Token lexNumber();
    Token lexString();
    Token lexCharLiteral();
    Token lexLineComment();
    Token lexBlockComment();
    Token lexPreprocessor();
    Token lexSymbol();           // single-char punctuation / operators

    // ── Character-stream primitives ────────────────────────────────────
    char cur()                  const noexcept; // current char (or '\0')
    char peekAt(int offset = 1) const noexcept; // lookahead without consuming
    char advance()              noexcept;        // consume and return current
    bool atEnd()                const noexcept;

    [[nodiscard]] bool isKeyword(const std::string& word) const noexcept;

    const std::string& m_src;
    std::size_t        m_pos  = 0;
    int                m_line = 1;  // 1-based
    int                m_col  = 1;  // 1-based
};

} // namespace cma
