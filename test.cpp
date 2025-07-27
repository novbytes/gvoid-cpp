#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <cctype>

enum class TokenType {
    PRINT,
    LPAREN,
    RPAREN,
    STRING
};

struct Token {
    TokenType type;
    std::string lexeme;  // The actual text of the token
    int line;
};

std::vector<Token> Tokenize(const std::string& source) {
    std::vector<Token> tokens;
    int line = 1;
    size_t current = 0;

    while (current < source.length()) {
        char c = source[current];

        // Skip whitespace
        if (isspace(c)) {
            if (c == '\n') line++;
            current++;
            continue;
        }

        // Handle identifiers/keywords
        if (isalpha(c)) {
            size_t start = current;
            while (current < source.length() && isalpha(source[current])) {
                current++;
            }

            std::string lexeme = source.substr(start, current - start);
            if (lexeme == "print") {
                tokens.push_back({TokenType::PRINT, lexeme, line});
            }
            continue;
        }

        // Handle parentheses
        if (c == '(') {
            tokens.push_back({TokenType::LPAREN, "(", line});
            current++;
            continue;
        }

        if (c == ')') {
            tokens.push_back({TokenType::RPAREN, ")", line});
            current++;
            continue;
        }

        // Handle strings
        if (c == '"') {
            size_t start = current;
            current++; // Skip opening quote
            while (current < source.length() && source[current] != '"') {
                if (source[current] == '\n') line++;
                current++;
            }

            if (current >= source.length()) {
                std::cerr << "Unterminated string at line " << line << std::endl;
                return {};
            }

            std::string lexeme = source.substr(start + 1, current - start - 1);
            tokens.push_back({TokenType::STRING, lexeme, line});
            current++; // Skip closing quote
            continue;
        }

        // If we get here, it's an unrecognized character
        std::cerr << "Unexpected character '" << c << "' at line " << line << std::endl;
        current++;
    }

    return tokens;
}

std::string token_to_cpp(const std::vector<Token>& tokens) {
    std::string cpp_code;
    bool expect_string = false;

    for (size_t i = 0; i < tokens.size(); i++) {
        const Token& token = tokens[i];

        switch (token.type) {
            case TokenType::PRINT:
                cpp_code += "std::cout << ";
                break;
            case TokenType::LPAREN:
                // In C++, parentheses aren't needed for simple cout statements
                break;
            case TokenType::RPAREN:
                // In C++, parentheses aren't needed for simple cout statements
                cpp_code += ";";
                break;
            case TokenType::STRING:
                cpp_code += "\"" + token.lexeme + "\"";
                // If there's more tokens after this string, add <<
                if (i + 1 < tokens.size() && tokens[i + 1].type != TokenType::RPAREN) {
                    cpp_code += " << ";
                }
                break;
        }
    }

    return cpp_code;
}

int main() {
    std::string code = "print(\"Hello world\")";
    auto tokens = Tokenize(code);

    // For debugging: print the tokens
    std::cout << "Tokens:\n";
    for (const auto& token : tokens) {
        std::cout << "  ";
        switch (token.type) {
            case TokenType::PRINT: std::cout << "PRINT"; break;
            case TokenType::LPAREN: std::cout << "LPAREN"; break;
            case TokenType::RPAREN: std::cout << "RPAREN"; break;
            case TokenType::STRING: std::cout << "STRING: " << token.lexeme; break;
        }
        std::cout << " (line " << token.line << ")\n";
    }

    std::string cpp_code = token_to_cpp(tokens);
    std::cout << "\nGenerated C++ code:\n" << cpp_code << std::endl;

    // Bonus: actually execute the generated code
    std::cout << "\nExecution output:\n";
    if (!cpp_code.empty()) {
        // This is just for demonstration - in a real compiler you'd generate a complete program
        if (cpp_code.back() != ';') cpp_code += ';';
        std::cout << "  (Would execute: " << cpp_code << ")\n";
    }

    return 0;
}