#pragma once

#include "tokens.hpp"
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <unordered_map>

class Lexer
{
public:
    explicit Lexer(const std::string &source)
        : m_source(source), m_currentPos(0), m_cline(1) {}

    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;

        while (!isAtEnd())
        {
            if (isspace(peek()))
            {
                if (peek() == '\n')
                {
                    m_cline++;
                }
                advance();
                continue;
            }

            if (peek() == '/' && peekNext() == '/')
            {
                skipComment();
                continue;
            }

            auto token = nextToken();
            if (token.has_value())
            {
                tokens.push_back(token.value());
            }
            else
            {
                tokens.push_back({TokenType::UNKNOWN, m_cline, std::string(1, peek())});
                advance();
            }
        }

        tokens.push_back({TokenType::END_OF_FILE, m_cline, {}});
        return tokens;
    }

private:
    const std::string m_source;
    size_t m_currentPos;
    int m_cline;

    static const std::unordered_map<std::string, TokenType> keywords;

    char peek() const
    {
        if (isAtEnd())
            return '\0';
        return m_source[m_currentPos];
    }

    char peekNext() const
    {
        if (m_currentPos + 1 >= m_source.size())
            return '\0';
        return m_source[m_currentPos + 1];
    }

    char advance()
    {
        if (isAtEnd())
            return '\0';
        return m_source[m_currentPos++];
    }

    bool isAtEnd() const
    {
        return m_currentPos >= m_source.size();
    }

    void skipComment()
    {
        while (peek() != '\n' && !isAtEnd())
        {
            advance();
        }
    }

    std::optional<Token> nextToken()
    {
        char c = peek();

        switch (c)
        {
        case ';':
            advance();
            return Token{TokenType::SEMICOLON, m_cline};
        case '(':
            advance();
            return Token{TokenType::LPAREN, m_cline};
        case ')':
            advance();
            return Token{TokenType::RPAREN, m_cline};
        case '{':
            advance();
            return Token{TokenType::LBRACE, m_cline};
        case '}':
            advance();
            return Token{TokenType::RBRACE, m_cline};
        case '[':
            advance();
            return Token{TokenType::LBRACKET, m_cline};
        case ']':
            advance();
            return Token{TokenType::RBRACKET, m_cline};
        case '+':
            if (peekNext() == '=')
            {
                advance();
                advance();
                return Token{TokenType::PLUS_EQ, m_cline};
            }
            if (peekNext() == '+')
            {
                advance();
                advance();
                return Token{TokenType::PLUS_PLUS, m_cline};
            }
            advance();
            return Token{TokenType::PLUS, m_cline};

        case '-':
            if (peekNext() == '=')
            {
                advance();
                advance();
                return Token{TokenType::MINUS_EQ, m_cline};
            }
            if (peekNext() == '-')
            {
                advance();
                advance();
                return Token{TokenType::MINUS_MINUS, m_cline};
            }
            if (peekNext() == '>')
            {
                advance();
                advance();
                return Token{TokenType::ARROW_RIGHT, m_cline};
            }
            advance();
            return Token{TokenType::MINUS, m_cline};
        case '@':
            advance();
            if (isalpha(peek()))
            {
                std::string directive;
                while (isalpha(peek()))
                {
                    directive += advance();
                }

                if (directive == "import")
                {
                    while (isspace(peek()))
                    {
                        if (peek() == '\n')
                            m_cline++;
                        advance();
                    }

                    std::string moduleName;
                    while (isalnum(peek()) || peek() == '_' || peek() == '.')
                    {
                        moduleName += advance();
                    }

                    return Token{TokenType::IMPORT, m_cline, moduleName};
                }
            }
            return Token{TokenType::AT, m_cline};

        case '*':
            if (peekNext() == '=')
            {
                advance();
                advance();
                return Token{TokenType::ASTER_EQ, m_cline};
            }
            advance();
            return Token{TokenType::ASTER, m_cline};

        case '/':
            if (peekNext() == '=')
            {
                advance();
                advance();
                return Token{TokenType::FSLASH_EQ, m_cline};
            }
            advance();
            return Token{TokenType::FSLASH, m_cline};

        case '%':
            if (peekNext() == '=')
            {
                advance();
                advance();
                return Token{TokenType::PERCENT_EQ, m_cline};
            }
            advance();
            return Token{TokenType::PERCENT, m_cline};
        case '<':
        {
            if (peekNext() == '<')
            {
                advance();
                advance();
                return Token{TokenType::STREAM_OUT, m_cline};
            }
            if (peekNext() == '-')
            {
                advance();
                advance();
                return Token{TokenType::ARROW_LEFT, m_cline};
            }
            advance();
            return Token{TokenType::LT, m_cline};
        }
        case '>':
            advance();
            return Token{TokenType::GT, m_cline};
        case '!':
            if (peekNext() == '=')
            {
                advance();
                advance();
                return Token{TokenType::BANG_EQ, m_cline};
            }
            advance();
            return Token{TokenType::NOT, m_cline};
        case '&':
            advance();
            if (peek() == '&')
            {
                advance();
                return Token{TokenType::LOGICAL_AND, m_cline};
            }
            return Token{TokenType::AND, m_cline};
        case '|':
        {
            if (peekNext() == '|')
            {
                advance();
                advance();
                return Token{TokenType::LOGICAL_OR, m_cline};
            }
            advance();
            return Token{TokenType::OR, m_cline};
        }

        case '^':
            advance();
            return Token{TokenType::XOR, m_cline};
        case '=':
            if (peekNext() == '=')
            {
                advance();
                advance();
                return Token{TokenType::EQ_EQ, m_cline};
            }
            if (peekNext() == '<')
            {
                advance();           
                advance();
                return Token{TokenType::LT_EQ, m_cline};     
            }
            if (peekNext() == '>')
            {
                advance();
                advance();
                return Token{TokenType::GT_EQ, m_cline};
            }
            advance();
            return Token{TokenType::ASSIGN, m_cline};
        case '~':
            advance();
            return Token{TokenType::BITWISE_NOT, m_cline};
        case '"':
            return stringLiteral();
        }

        if (isdigit(c))
        {
            return numberLiteral();
        }

        if (isalpha(c) || c == '_')
        {
            return identifier();
        }

        return std::nullopt;
    }

    Token stringLiteral()
    {
        advance();
        std::string value;
        while (peek() != '"' && !isAtEnd())
        {
            if (peek() == '\n')
                m_cline++;
            value += advance();
        }

        if (isAtEnd())
        {
            return {TokenType::UNKNOWN, m_cline, value};
        }

        advance();
        return {TokenType::STRING_LIT, m_cline, value};
    }

    Token numberLiteral()
    {
        std::string value;
        while (isdigit(peek()))
        {
            value += advance();
        }

        if (peek() == '.' && isdigit(peekNext()))
        {
            value += advance();
            while (isdigit(peek()))
            {
                value += advance();
            }
        }

        return {TokenType::NUMBER, m_cline, value};
    }

    Token identifier()
    {
        std::string value;
        while (isalnum(peek()) || peek() == '_')
        {
            value += advance();
        }

        auto it = keywords.find(value);
        if (it != keywords.end())
        {
            return {it->second, m_cline};
        }

        return {TokenType::IDENTIFIER, m_cline, value};
    }
};

const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"return", TokenType::RETURN},
    {"import", TokenType::IMPORT},
    {"num", TokenType::KEYWORD_VAR_NUM},
    {"str", TokenType::KEYWORD_VAR_STR},
    {"arr", TokenType::KEYWORD_VAR_ARR},
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"do", TokenType::DO},
    {"for", TokenType::FOR},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"print", TokenType::PRINT},
    {"func", TokenType::FUNCTION},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE}
};