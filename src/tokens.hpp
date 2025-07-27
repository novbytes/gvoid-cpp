#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

enum class TokenType
{
    //--(Keywords
    PRINT,
    RETURN,
    KEYWORD_VAR_NUM, // num
    KEYWORD_VAR_STR, // str
    KEYWORD_VAR_ARR, // arr<num or str> 
    ARROW_RIGHT,
    ARROW_LEFT,
    FUNCTION,
    IMPORT,
    IF,
    ELIF,
    ELSE,
    WHILE,
    DO,
    FOR,
    BREAK,
    CONTINUE,

    //--(Literals
    STRING_LIT,
    NUMBER,
    IDENTIFIER,

    //--(Symbols & Operators
    SEMICOLON, // ;
    COMMA,     // ,
    ASSIGN,    // =
    PLUS,      // +
    PLUS_PLUS, // ++
    MINUS_MINUS, // --
    MINUS,     // -
    ASTER,     // *
    FSLASH,    // /
    PERCENT,   // %
    AT,        // @
    LT,        // <
    GT,        // >
    LT_EQ,
    GT_EQ,
    BITWISE_NOT,
    STREAM_IN,
    TRUE,
    FALSE,
    NOT,       // !
    AND,       // &
    OR,        // |
    XOR,       // ^
    DOLLAR,    //

    LOGICAL_OR, // ||
    LOGICAL_AND, // &&
    STREAM_OUT, // <<

    //-(Compound assignment operators
    PLUS_EQ,    // +=
    MINUS_EQ,   // -=
    ASTER_EQ,   // *=
    FSLASH_EQ,  // /=
    PERCENT_EQ, // %=

    //-(Comparison operators
    EQ_EQ,   // ==
    BANG_EQ, // !=

    //--(Grouping
    LPAREN, // (
    RPAREN, // )
    LBRACE, // {
    RBRACE, // }
    LBRACKET, // [
    RBRACKET, // ]

    //--(Misc
    DOUBLE_FLASH, // //
    END_OF_FILE,
    UNKNOWN
};

inline std::string to_string(const TokenType token)
{
    switch (token)
    {
    case TokenType::RETURN:
        return "return";
    case TokenType::PRINT:
        return "print";
    case TokenType::KEYWORD_VAR_NUM:
        return "num";
    case TokenType::KEYWORD_VAR_STR:
        return "str";
    case TokenType::KEYWORD_VAR_ARR:
        return "arr";
    case TokenType::FUNCTION:
        return "func";
    case TokenType::IMPORT:
        return "import";
    case TokenType::IF:
        return "if";
    case TokenType::ELIF:
        return "elif";
    case TokenType::ELSE:
        return "else";
    case TokenType::WHILE:
        return "while";
    case TokenType::LT_EQ:
        return "<=";
    case TokenType::GT_EQ:
        return "=>";
    case TokenType::TRUE:
        return "true";
    case TokenType::FALSE:
        return "false";
    case TokenType::DO:
        return "do";
    case TokenType::FOR:
        return "for";
    case TokenType::BREAK:
        return "break";
    case TokenType::CONTINUE:
        return "continue";
    case TokenType::STRING_LIT:
        return "string_lit";
    case TokenType::NUMBER:
        return "number";
    case TokenType::IDENTIFIER:
        return "identifier";
    case TokenType::DOLLAR:
        return "$";
    case TokenType::SEMICOLON:
        return ";";
    case TokenType::COMMA:
        return ",";
    case TokenType::ASSIGN:
        return "=";
    case TokenType::PLUS:
        return "+";
    case TokenType::PLUS_PLUS:
        return "++";
    case TokenType::MINUS_MINUS:
        return "--";
    case TokenType::MINUS:
        return "-";
    case TokenType::ASTER:
        return "*";
    case TokenType::FSLASH:
        return "/";
    case TokenType::AT:
        return "@";
    case TokenType::PERCENT:
        return "%";
    case TokenType::LT:
        return "<";
    case TokenType::GT:
        return ">";
    case TokenType::ARROW_LEFT:
        return "<-";
    case TokenType::ARROW_RIGHT:
        return "->";
    case TokenType::NOT:
        return "!";
    case TokenType::AND:
        return "&";
    case TokenType::OR:
        return "|";
    case TokenType::XOR:
        return "^";
    case TokenType::LOGICAL_OR:
        return "||";
    case TokenType::LOGICAL_AND:
        return "&&";
    case TokenType::STREAM_OUT:
        return "<<";
    case TokenType::PLUS_EQ:
        return "+=";
    case TokenType::MINUS_EQ:
        return "-=";
    case TokenType::ASTER_EQ:
        return "*=";
    case TokenType::FSLASH_EQ:
        return "/=";
    case TokenType::PERCENT_EQ:
        return "%=";
    case TokenType::EQ_EQ:
        return "==";
    case TokenType::BANG_EQ:
        return "!=";
    case TokenType::LPAREN:
        return "(";
    case TokenType::RPAREN:
        return ")";
    case TokenType::LBRACE:
        return "{";
    case TokenType::RBRACE:
        return "}";
    case TokenType::LBRACKET:
        return "[";
    case TokenType::RBRACKET:
        return "]";
    case TokenType::DOUBLE_FLASH:
        return "//";
    case TokenType::END_OF_FILE:
        return "end_of_file";
    case TokenType::UNKNOWN:
        return "unknown";
    default:
        return "invalid_token";
    }
}

/* 
currently unused i will use this function later

reference : 
@orosmatthew hydrogen-cpp - https://github.com/orosmatthew/hydrogen-cpp

*/
inline std::optional<int> bin_prec(const TokenType type)
{
    switch (type)
    {
        case TokenType::ASSIGN:
        case TokenType::PLUS_EQ:
        case TokenType::MINUS_EQ:
        case TokenType::ASTER_EQ:
        case TokenType::FSLASH_EQ:
        case TokenType::PERCENT_EQ:
            return 1;

        // Stream operator
        case TokenType::STREAM_OUT:
            return 2;

        // Logical OR (||)
        case TokenType::LOGICAL_OR:
            return 3;

        // Logical AND (&&) 
        case TokenType::LOGICAL_AND:
            return 4;

        // Bitwise OR (|)
        case TokenType::OR:
            return 5;

        // Bitwise XOR (^)
        case TokenType::XOR:
            return 6;

        // Bitwise AND (&)
        case TokenType::AND:
            return 7;

        // Equality comparisons (==, !=)
        case TokenType::EQ_EQ:
        case TokenType::BANG_EQ:
            return 8;

        // Relational comparisons (<, >)
        case TokenType::LT:
        case TokenType::GT:
            return 9;

        // Additive (+, -)
        case TokenType::PLUS:
        case TokenType::MINUS:
            return 10;

        // Multiplicative (*, /, %)
        case TokenType::ASTER:
        case TokenType::FSLASH:
        case TokenType::PERCENT:
            return 11;

        default:
            return {};
    }
}

struct Token
{
    TokenType type;
    int line;
    std::optional<std::string> value{};
};