#pragma once

#include <string>

namespace cma {

// Every character sequence the Lexer recognises maps to one of these.
// Keeping them fine-grained lets the Parser make precise decisions
// without re-examining raw text.
enum class TokenType {
    // Identifiers / keywords
    KEYWORD,
    IDENTIFIER,

    // Literals — content is opaque; we just need to skip them
    STRING_LITERAL,
    CHAR_LITERAL,
    NUMBER_LITERAL,

    // Delimiters — each gets its own type so the Parser never parses
    // a char value to decide what a brace means
    OPEN_BRACE,    // {
    CLOSE_BRACE,   // }
    OPEN_PAREN,    // (
    CLOSE_PAREN,   // )
    OPEN_BRACKET,  // [
    CLOSE_BRACKET, // ]
    SEMICOLON,     // ;

    // Comments — carried as tokens so the Parser can classify lines
    // and count TODO/FIXME markers without a second source scan
    LINE_COMMENT,   // //...
    BLOCK_COMMENT,  // /* ... */

    // Preprocessor — kept separate so #include counting is trivial
    PREPROCESSOR,   // # ...

    // Everything else
    OPERATOR,       // + - * / % = < > ! & | ^ ~ ? : . ->
    PUNCTUATION,    // , ; @ etc.
    NEWLINE,        // explicit '\n' — needed for line classification
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType   type  = TokenType::UNKNOWN;
    std::string value;
    int         line  = 0;   // 1-based source line
    int         col   = 0;   // 1-based column
};

// ── Keyword category predicates ────────────────────────────────────────────
// Defined inline here so every consumer can use them without a .cpp

inline bool isLoopKeyword(const Token& t) noexcept {
    return t.type == TokenType::KEYWORD &&
           (t.value == "for" || t.value == "while" || t.value == "do");
}

inline bool isCondKeyword(const Token& t) noexcept {
    return t.type == TokenType::KEYWORD &&
           (t.value == "if" || t.value == "switch");
}

// class, struct, enum, namespace — all introduce a named scope block
inline bool isClassLike(const Token& t) noexcept {
    return t.type == TokenType::KEYWORD &&
           (t.value == "class"     || t.value == "struct" ||
            t.value == "enum"      || t.value == "namespace");
}

inline bool isExceptionKw(const Token& t) noexcept {
    return t.type == TokenType::KEYWORD &&
           (t.value == "try" || t.value == "catch");
}

} // namespace cma
