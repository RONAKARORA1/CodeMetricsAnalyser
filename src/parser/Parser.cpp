#include "parser/Parser.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace cma {

// ── FileMetrics derived helpers ────────────────────────────────────────────
// Defined here (not in the header) because they require FunctionInfo to be
// a complete type — which it is by the time this translation unit compiles.

const FunctionInfo* FileMetrics::longestFunction() const noexcept {
    if (functions.empty()) return nullptr;
    return &*std::max_element(
        functions.begin(), functions.end(),
        [](const FunctionInfo& a, const FunctionInfo& b) {
            return a.lineCount() < b.lineCount();
        });
}

double FileMetrics::avgFunctionLength() const noexcept {
    if (functions.empty()) return 0.0;
    double total = 0.0;
    for (const auto& fn : functions) total += fn.lineCount();
    return total / static_cast<double>(functions.size());
}

// ── Constructor ────────────────────────────────────────────────────────────
Parser::Parser(const std::vector<Token>& tokens, int totalLines)
    : m_tokens(tokens)
    , m_totalLines(totalLines)
{
    // Pre-mark every line BLANK; the classification pass will upgrade lines
    // that contain comment or code tokens
    m_lineTypes.assign(static_cast<std::size_t>(totalLines), LineType::BLANK);
    m_result.totalLines = totalLines;
}

// ── Public API ─────────────────────────────────────────────────────────────
FileMetrics Parser::analyze() {
    classifyLines();
    walkTokens();

    // Tally the per-line classification into the final counts
    for (const auto& lt : m_lineTypes) {
        switch (lt) {
            case LineType::BLANK:   ++m_result.blankLines;   break;
            case LineType::COMMENT: ++m_result.commentLines; break;
            case LineType::CODE:    ++m_result.codeLines;    break;
        }
    }

    m_result.maxNestingDepth = m_maxDepth;
    return m_result;
}

// ── Pass 1: line classification ────────────────────────────────────────────
// Priority rule: CODE > COMMENT > BLANK
// A line containing both a comment and real code is CODE (mixed lines do exist).
void Parser::classifyLines() {
    const auto mark = [&](int srcLine, LineType lt) {
        const auto idx = static_cast<std::size_t>(srcLine - 1);
        if (idx >= m_lineTypes.size()) return;
        // Only upgrade; never downgrade a line that was already marked CODE
        if (lt == LineType::CODE ||
            (lt == LineType::COMMENT && m_lineTypes[idx] == LineType::BLANK)) {
            m_lineTypes[idx] = lt;
        }
    };

    for (const auto& tok : m_tokens) {
        if (tok.type == TokenType::END_OF_FILE ||
            tok.type == TokenType::NEWLINE)
            continue;

        if (tok.type == TokenType::LINE_COMMENT) {
            mark(tok.line, LineType::COMMENT);
            m_result.todoCount += countTodos(tok.value);

        } else if (tok.type == TokenType::BLOCK_COMMENT) {
            // Span every line inside the block comment
            int line = tok.line;
            for (char c : tok.value) {
                mark(line, LineType::COMMENT);
                if (c == '\n') ++line;
            }
            m_result.todoCount += countTodos(tok.value);

        } else if (tok.type == TokenType::PREPROCESSOR) {
            mark(tok.line, LineType::CODE);
            // Count #include directives for the dependency metric
            if (tok.value.find("#include") != std::string::npos)
                ++m_result.includeCount;

        } else {
            // Every other token (keyword, identifier, literal, delimiter)
            // marks its line as source code
            mark(tok.line, LineType::CODE);
        }
    }
}

// ── Pass 2: structural and complexity analysis ─────────────────────────────
void Parser::walkTokens() {
    const std::size_t n = m_tokens.size();

    for (std::size_t i = 0; i < n; ++i) {
        const Token& tok = m_tokens[i];

        switch (tok.type) {

        // ── Brace depth tracking ─────────────────────────────────────────
        case TokenType::OPEN_BRACE:
            ++m_braceDepth;
            m_maxDepth = std::max(m_maxDepth, m_braceDepth);
            break;

        case TokenType::CLOSE_BRACE:
            // Check for function closure BEFORE decrementing —
            // the closing brace is still at m_fnBraceDepth at this point
            if (m_inFunction && m_braceDepth == m_fnBraceDepth) {
                m_currentFn.endLine = tok.line;
                m_result.functions.push_back(m_currentFn);
                m_inFunction = false;
            }
            if (m_braceDepth > 0) --m_braceDepth;
            break;

        // ── Keyword dispatch ─────────────────────────────────────────────
        case TokenType::KEYWORD:
            handleKeyword(i);
            break;

        // ── Identifier: possible function start or variable decl ─────────
        case TokenType::IDENTIFIER:
            if (i + 1 < n && m_tokens[i + 1].type == TokenType::OPEN_PAREN)
                tryBeginFunction(i);
            else
                handleVariableDecl(i);
            break;

        // ── Logical operators and ternary → cyclomatic complexity ────────
        case TokenType::OPERATOR:
            if (tok.value == "?") {
                ++m_result.cyclomaticComplexity;
            }
            // && and || are emitted as two consecutive single-char OPERATOR tokens;
            // detect the pair here and skip the second to avoid double-counting
            else if ((tok.value == "&" || tok.value == "|") &&
                      i + 1 < n &&
                      m_tokens[i + 1].type  == TokenType::OPERATOR &&
                      m_tokens[i + 1].value == tok.value) {
                ++m_result.cyclomaticComplexity;
                ++i; // skip the second & or |
            }
            break;

        default:
            break;
        }
    }
}

// ── Keyword handler ────────────────────────────────────────────────────────
void Parser::handleKeyword(std::size_t idx) {
    const Token& tok = m_tokens[idx];

    if (isLoopKeyword(tok)) {
        ++m_result.loopCount;
        ++m_result.cyclomaticComplexity;

    } else if (isCondKeyword(tok)) {
        ++m_result.conditionCount;
        ++m_result.cyclomaticComplexity;

    } else if (tok.value == "case") {
        // Each case label is an independent execution path
        ++m_result.cyclomaticComplexity;

    } else if (tok.value == "catch") {
        ++m_result.cyclomaticComplexity;

    } else if (tok.value == "try") {
        ++m_result.tryCatchCount;

    } else if (isClassLike(tok)) {
        // Grab the name token that follows class/struct/enum/namespace
        const std::size_t nameIdx = idx + 1;
        if (nameIdx < m_tokens.size() &&
            m_tokens[nameIdx].type == TokenType::IDENTIFIER) {
            ClassInfo ci;
            ci.name = m_tokens[nameIdx].value;
            ci.line = tok.line;
            if      (tok.value == "class")     ci.kind = ClassInfo::Kind::CLASS;
            else if (tok.value == "struct")    ci.kind = ClassInfo::Kind::STRUCT;
            else if (tok.value == "enum")      ci.kind = ClassInfo::Kind::ENUM;
            else                               ci.kind = ClassInfo::Kind::NAMESPACE;
            m_result.classes.push_back(ci);
        }
    }
}

// ── Function definition detection ─────────────────────────────────────────
// Pattern: IDENTIFIER ( params ) [specifiers] {
// Only checked at brace depth 0 (global) or 1 (class body).
void Parser::tryBeginFunction(std::size_t identIdx) {
    // Don't nest: we track at most one function at a time
    if (m_inFunction || m_braceDepth > 1) return;

    const std::size_t openParenIdx  = identIdx + 1;
    const std::size_t closeParenIdx = findMatchingParen(openParenIdx);
    if (closeParenIdx >= m_tokens.size()) return;

    const std::size_t bodyIdx = skipTrailingSpecifiers(closeParenIdx + 1);
    if (bodyIdx >= m_tokens.size()) return;
    if (m_tokens[bodyIdx].type != TokenType::OPEN_BRACE) return;

    // The OPEN_BRACE at bodyIdx will be processed in the main loop shortly
    // and will increment m_braceDepth — so the function body lives at depth+1
    m_inFunction   = true;
    m_fnBraceDepth = m_braceDepth + 1;
    m_currentFn    = {};
    m_currentFn.name      = m_tokens[identIdx].value;
    m_currentFn.startLine = m_tokens[identIdx].line;
}

// ── Variable declaration heuristic ────────────────────────────────────────
// Counts an IDENTIFIER that immediately follows a primitive-type keyword
// and is NOT followed by '(' (which would make it a function).
// Only catches: int x, double y = 3.0, auto z = ..., etc.
// Does NOT catch: std::string s, vector<int> v, user-defined types, etc.
// Documented as approximate in ParseResult.h.
void Parser::handleVariableDecl(std::size_t identIdx) {
    static const std::unordered_set<std::string> kPrimitiveTypes = {
        "int","long","short","char","bool","float","double",
        "unsigned","signed","auto","wchar_t","size_t","void"
    };

    if (identIdx == 0) return;

    // Walk backward past any const / pointer / reference modifiers
    std::size_t prev = identIdx - 1;
    while (prev > 0 &&
           m_tokens[prev].type == TokenType::OPERATOR &&
           (m_tokens[prev].value == "*" || m_tokens[prev].value == "&")) {
        --prev;
    }
    // Skip 'const' modifier
    if (m_tokens[prev].type == TokenType::KEYWORD &&
        m_tokens[prev].value == "const" && prev > 0) {
        --prev;
    }

    if (m_tokens[prev].type != TokenType::KEYWORD) return;
    if (!kPrimitiveTypes.count(m_tokens[prev].value)) return;

    // Exclude: primitive-type keyword immediately before function name
    const std::size_t next = identIdx + 1;
    if (next < m_tokens.size() &&
        m_tokens[next].type == TokenType::OPEN_PAREN) return;

    ++m_result.variableCount;
}

// ── Scanning helpers ───────────────────────────────────────────────────────
std::size_t Parser::findMatchingParen(std::size_t openIdx) const {
    if (openIdx >= m_tokens.size() ||
        m_tokens[openIdx].type != TokenType::OPEN_PAREN)
        return m_tokens.size();

    int depth = 1;
    for (std::size_t i = openIdx + 1; i < m_tokens.size(); ++i) {
        if (m_tokens[i].type == TokenType::OPEN_PAREN)  ++depth;
        if (m_tokens[i].type == TokenType::CLOSE_PAREN) { if (--depth == 0) return i; }
        if (m_tokens[i].type == TokenType::END_OF_FILE) break;
    }
    return m_tokens.size(); // unmatched
}

// Specifiers that legally appear between ')' and '{' in a C++ function
// signature.  We skip them all to reach the opening brace of the body.
std::size_t Parser::skipTrailingSpecifiers(std::size_t i) const {
    static const std::unordered_set<std::string> kSpecifiers = {
        "const","noexcept","override","final","volatile","mutable","requires"
    };

    const std::size_t n = m_tokens.size();
    int parenDepth = 0; // track noexcept(expr)

    while (i < n) {
        const Token& t = m_tokens[i];

        if (t.type == TokenType::NEWLINE) { ++i; continue; }

        // Track parens inside noexcept(bool-expr) or requires(constraint)
        if (t.type == TokenType::OPEN_PAREN)  { ++parenDepth; ++i; continue; }
        if (t.type == TokenType::CLOSE_PAREN) { --parenDepth; ++i; continue; }
        if (parenDepth > 0) { ++i; continue; }

        if (t.type == TokenType::KEYWORD && kSpecifiers.count(t.value)) {
            ++i; continue;
        }

        // Constructor initializer list: Foo() : m_x(0), m_y(1) {
        // After ')', a lone ':' (not '::') can only be a member-init separator.
        if (t.type == TokenType::OPERATOR && t.value == ":") {
            // Skip forward until we hit the opening brace
            while (i < n && m_tokens[i].type != TokenType::OPEN_BRACE) ++i;
            break;
        }

        // Trailing return type: auto f() -> RetType {
        if (t.type == TokenType::OPERATOR && t.value == "-" &&
            i + 1 < n && m_tokens[i + 1].value == ">") {
            i += 2; // skip '-' and '>'
            // Skip the return-type tokens until '{' or ';'
            while (i < n) {
                const auto& rt = m_tokens[i];
                if (rt.type == TokenType::OPEN_BRACE ||
                    rt.type == TokenType::SEMICOLON)
                    break;
                ++i;
            }
            continue;
        }

        break; // reached a token that isn't a specifier — done
    }
    return i;
}

int Parser::countTodos(const std::string& text) const {
    int count = 0;
    for (std::size_t pos = 0;
         (pos = text.find("TODO", pos)) != std::string::npos;
         pos += 4) { ++count; }
    for (std::size_t pos = 0;
         (pos = text.find("FIXME", pos)) != std::string::npos;
         pos += 5) { ++count; }
    return count;
}

} // namespace cma
