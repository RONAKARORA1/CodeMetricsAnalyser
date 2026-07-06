#include "lexer/Lexer.h"

#include <cctype>
#include <unordered_set>

namespace cma {

// ── C++ keyword table ──────────────────────────────────────────────────────
// Kept as an unordered_set<string> for O(1) lookup.
// 'final' and 'override' are context-sensitive identifiers in C++11+;
// including them here keeps function-definition detection clean.
static const std::unordered_set<std::string> kKeywords = {
    "alignas","alignof","auto",
    "bool","break",
    "case","catch","char","char8_t","char16_t","char32_t","class",
    "const","constexpr","consteval","constinit","const_cast","continue",
    "decltype","default","delete","do","double","dynamic_cast",
    "else","enum","explicit","export","extern",
    "false","final","float","for","friend",
    "goto",
    "if","inline","int",
    "long",
    "mutable",
    "namespace","new","noexcept","nullptr",
    "operator","override",
    "private","protected","public",
    "register","reinterpret_cast","requires","return",
    "short","signed","sizeof","static","static_assert","static_cast",
    "struct","switch",
    "template","this","thread_local","throw","true","try","typedef",
    "typeid","typename",
    "union","unsigned","using",
    "virtual","void","volatile",
    "wchar_t","while"
};

// ── Constructor ────────────────────────────────────────────────────────────
Lexer::Lexer(const std::string& source)
    : m_src(source) {}

// ── Public API ─────────────────────────────────────────────────────────────
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(m_src.size() / 4); // rough heuristic: ~4 chars per token

    while (!atEnd()) {
        const char c = cur();

        // Newlines are emitted as explicit tokens.
        // The Parser uses them to classify lines without a second source pass.
        if (c == '\n') {
            tokens.push_back({TokenType::NEWLINE, "\n", m_line, m_col});
            advance();
            continue;
        }

        // Other whitespace is discarded — it carries no metric information
        if (std::isspace(static_cast<unsigned char>(c))) {
            advance();
            continue;
        }

        // Two-char lookahead dispatch — order matters:
        // '//' must be checked before '/' alone
        if (c == '/' && peekAt() == '/') { tokens.push_back(lexLineComment());  continue; }
        if (c == '/' && peekAt() == '*') { tokens.push_back(lexBlockComment()); continue; }
        if (c == '#')                    { tokens.push_back(lexPreprocessor());  continue; }
        if (c == '"')                    { tokens.push_back(lexString());        continue; }
        if (c == '\'')                   { tokens.push_back(lexCharLiteral());   continue; }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(lexNumber());
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(lexIdentifierOrKeyword());
            continue;
        }

        tokens.push_back(lexSymbol());
    }

    tokens.push_back({TokenType::END_OF_FILE, "", m_line, m_col});
    return tokens;
}

// ── Token-category lexers ──────────────────────────────────────────────────

Token Lexer::lexIdentifierOrKeyword() {
    const int sl = m_line, sc = m_col;
    std::string value;

    while (!atEnd() &&
           (std::isalnum(static_cast<unsigned char>(cur())) || cur() == '_')) {
        value += advance();
    }

    const TokenType type =
        isKeyword(value) ? TokenType::KEYWORD : TokenType::IDENTIFIER;
    return {type, std::move(value), sl, sc};
}

Token Lexer::lexNumber() {
    const int sl = m_line, sc = m_col;
    std::string value;

    // Absorb digits, hex chars, separators, decimal point, and
    // the +/- that follows an exponent marker in scientific notation
    while (!atEnd()) {
        const char c = cur();
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.') {
            value += advance();
        } else if ((c == '+' || c == '-') && !value.empty() &&
                   (value.back() == 'e' || value.back() == 'E')) {
            value += advance();
        } else {
            break;
        }
    }
    return {TokenType::NUMBER_LITERAL, std::move(value), sl, sc};
}

Token Lexer::lexString() {
    const int sl = m_line, sc = m_col;
    std::string value;

    value += advance(); // opening "
    while (!atEnd() && cur() != '"') {
        if (cur() == '\\') {
            value += advance();                       // backslash
            if (!atEnd()) value += advance();         // escaped char
        } else if (cur() == '\n') {
            break;  // unterminated literal — recover gracefully
        } else {
            value += advance();
        }
    }
    if (!atEnd()) value += advance(); // closing "
    return {TokenType::STRING_LITERAL, std::move(value), sl, sc};
}

Token Lexer::lexCharLiteral() {
    const int sl = m_line, sc = m_col;
    std::string value;

    value += advance(); // opening '
    while (!atEnd() && cur() != '\'') {
        if (cur() == '\\') {
            value += advance();
            if (!atEnd()) value += advance();
        } else if (cur() == '\n') {
            break;
        } else {
            value += advance();
        }
    }
    if (!atEnd()) value += advance(); // closing '
    return {TokenType::CHAR_LITERAL, std::move(value), sl, sc};
}

Token Lexer::lexLineComment() {
    const int sl = m_line, sc = m_col;
    std::string value;
    // Consume until end-of-line; do NOT consume the '\n' itself —
    // the main loop will emit it as a NEWLINE token
    while (!atEnd() && cur() != '\n') value += advance();
    return {TokenType::LINE_COMMENT, std::move(value), sl, sc};
}

Token Lexer::lexBlockComment() {
    const int sl = m_line, sc = m_col;
    std::string value;

    value += advance(); // '/'
    value += advance(); // '*'
    while (!atEnd()) {
        if (cur() == '*' && peekAt() == '/') {
            value += advance(); // '*'
            value += advance(); // '/'
            break;
        }
        // advance() updates m_line when it hits '\n', so multi-line
        // block comments automatically track line numbers
        value += advance();
    }
    // Note: unclosed block comment (atEnd() with no closing */) is
    // handled by emitting whatever we consumed — no throw
    return {TokenType::BLOCK_COMMENT, std::move(value), sl, sc};
}

Token Lexer::lexPreprocessor() {
    const int sl = m_line, sc = m_col;
    std::string value;

    while (!atEnd()) {
        if (cur() == '\n') {
            // Backslash-newline continues the directive onto the next line
            if (!value.empty() && value.back() == '\\') {
                value.back() = ' '; // normalise continuation
                advance();          // skip '\n'
            } else {
                break; // directive ends at newline
            }
        } else {
            value += advance();
        }
    }
    return {TokenType::PREPROCESSOR, std::move(value), sl, sc};
}

Token Lexer::lexSymbol() {
    const int sl = m_line, sc = m_col;
    const char c = advance();
    const std::string val(1, c);

    switch (c) {
        case '{': return {TokenType::OPEN_BRACE,    val, sl, sc};
        case '}': return {TokenType::CLOSE_BRACE,   val, sl, sc};
        case '(': return {TokenType::OPEN_PAREN,    val, sl, sc};
        case ')': return {TokenType::CLOSE_PAREN,   val, sl, sc};
        case '[': return {TokenType::OPEN_BRACKET,  val, sl, sc};
        case ']': return {TokenType::CLOSE_BRACKET, val, sl, sc};
        case ';': return {TokenType::SEMICOLON,     val, sl, sc};
        // Operators that contribute to cyclomatic complexity (& | ? :)
        // are kept as single characters so the Parser can detect && and ||
        // as two consecutive same-character operators
        case '+': case '-': case '*': case '/':
        case '%': case '=': case '<': case '>':
        case '!': case '&': case '|': case '^':
        case '~': case '?': case ':': case '.':
            return {TokenType::OPERATOR, val, sl, sc};
        default:
            return {TokenType::PUNCTUATION, val, sl, sc};
    }
}

// ── Character-stream primitives ────────────────────────────────────────────

char Lexer::cur() const noexcept {
    return atEnd() ? '\0' : m_src[m_pos];
}

char Lexer::peekAt(int offset) const noexcept {
    const auto idx = static_cast<std::size_t>(
        static_cast<std::ptrdiff_t>(m_pos) + offset);
    return (idx < m_src.size()) ? m_src[idx] : '\0';
}

char Lexer::advance() noexcept {
    const char c = m_src[m_pos++];
    if (c == '\n') { ++m_line; m_col = 1; }
    else           { ++m_col; }
    return c;
}

bool Lexer::atEnd() const noexcept {
    return m_pos >= m_src.size();
}

bool Lexer::isKeyword(const std::string& word) const noexcept {
    return kKeywords.count(word) != 0;
}

} // namespace cma
