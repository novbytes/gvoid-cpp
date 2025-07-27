#pragma once

#include "lexer.hpp"
#include "ast.hpp"
#include <memory>
#include <vector>
#include <stdexcept>

class Parser
{
public:
    explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)), m_current(0) {}

    std::vector<AST::StmtPtr> parse()
    {
        std::vector<AST::StmtPtr> statements;
        while (!isAtEnd())
        {
            statements.push_back(declaration());
        }
        return statements;
    }

private:
    std::vector<Token> m_tokens;
    size_t m_current;

    bool isAtEnd() const
    {
        return peek().type == TokenType::END_OF_FILE;
    }

    const Token &peek() const
    {
        return m_tokens[m_current];
    }

    const Token &previous() const
    {
        return m_tokens[m_current - 1];
    }

    bool check(TokenType type) const
    {
        if (isAtEnd())
            return false;
        return peek().type == type;
    }

    Token advance()
    {
        if (!isAtEnd())
            m_current++;
        return previous();
    }

    bool match(TokenType type)
    {
        if (check(type))
        {
            advance();
            return true;
        }
        return false;
    }

    bool match(const std::initializer_list<TokenType> &types)
    {
        for (auto type : types)
        {
            if (check(type))
            {
                advance();
                return true;
            }
        }
        return false;
    }

    Token consume(TokenType type, const std::string &message)
    {
        if (check(type))
            return advance();
        throw parseError(peek(), message);
    }

    std::runtime_error parseError(const Token &token, const std::string &message)
    {
        return std::runtime_error("[Line " + std::to_string(token.line) + "] Error: " + message);
    }

    void synchronize()
    {
        advance();
        while (!isAtEnd())
        {
            if (previous().type == TokenType::SEMICOLON)
                return;

            switch (peek().type)
            {
            case TokenType::FUNCTION:
            case TokenType::KEYWORD_VAR_NUM:
            case TokenType::KEYWORD_VAR_STR:
            case TokenType::KEYWORD_VAR_ARR:
            case TokenType::IMPORT:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::FOR:
            case TokenType::RETURN:
                return;
            default:
                advance();
            }
        }
    }

    AST::StmtPtr declaration()
    {
        try
        {
            if (match(TokenType::FUNCTION))
                return function();
            if (match(TokenType::KEYWORD_VAR_NUM))
                return numVarDeclaration();
            if (match(TokenType::KEYWORD_VAR_STR))
                return strVarDeclaration();
            if (match(TokenType::KEYWORD_VAR_ARR))
                return arrVarDeclaration();
            if (match(TokenType::IMPORT))
                return importStatement();
            return statement();
        }
        catch (const std::runtime_error &error)
        {
            synchronize();
            throw;
        }
    }

    AST::StmtPtr statement()
    {
        if (match(TokenType::IF))
            return ifStatement();
        if (match(TokenType::WHILE))
            return whileStatement();
        if (match(TokenType::FOR))
            return forStatement();
        if (match(TokenType::LBRACE))
            return block();
        if (match(TokenType::RETURN))
            return returnStatement();
        if (match(TokenType::PRINT))
            return printStatement();
        return expressionStatement();
    }

    AST::StmtPtr function()
    {
        std::string name = consume(TokenType::IDENTIFIER, "Expect function name").value.value();
        consume(TokenType::LPAREN, "Expect '(' after function name");

        std::vector<std::string> parameters;
        if (!check(TokenType::RPAREN))
        {
            do
            {
                parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name").value.value());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RPAREN, "Expect ')' after parameters");
        consume(TokenType::LBRACE, "Expect '{' before function body");

        auto body = block();
        return std::make_unique<AST::FunctionStmt>(
            std::move(name),
            std::move(parameters),
            std::move(body),
            previous().line);
    }

    AST::StmtPtr importStatement()
    {
        int line = previous().line;
        std::string moduleName;

        if (previous().value.has_value())
        {
            moduleName = previous().value.value();
        }
        else if (match(TokenType::STRING_LIT))
        {
            moduleName = previous().value.value();
        }
        else
        {
            throw parseError(peek(), "Expect module name after import");
        }

        consume(TokenType::SEMICOLON, "Expect ';' after import statement");
        return std::make_unique<AST::ImportStmt>(moduleName, line);
    }

    AST::StmtPtr numVarDeclaration()
    {
        return typedVarDeclaration(TokenType::KEYWORD_VAR_NUM);
    }

    AST::StmtPtr strVarDeclaration()
    {
        return typedVarDeclaration(TokenType::KEYWORD_VAR_STR);
    }

    AST::StmtPtr arrVarDeclaration()
    {
        return typedVarDeclaration(TokenType::KEYWORD_VAR_ARR);
    }

    AST::StmtPtr typedVarDeclaration(TokenType type)
    {
        int line = previous().line;
        std::string typeName;

        switch (type)
        {
        case TokenType::KEYWORD_VAR_NUM:
            typeName = "num";
            break;
        case TokenType::KEYWORD_VAR_STR:
            typeName = "str";
            break;
        case TokenType::KEYWORD_VAR_ARR:
            typeName = "arr";
            break;
        default:
            throw parseError(previous(), "Invalid variable type");
        }

        std::string name = consume(TokenType::IDENTIFIER, "Expect variable name").value.value();

        AST::ExprPtr initializer = nullptr;
        if (match(TokenType::ASSIGN))
        {
            initializer = expression();
        }

        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");
        return std::make_unique<AST::VarDeclStmt>(typeName, name, std::move(initializer), line);
    }

    AST::StmtPtr ifStatement()
    {
        int line = previous().line;
        consume(TokenType::LPAREN, "Expect '(' after 'if'");
        auto condition = expression();
        consume(TokenType::RPAREN, "Expect ')' after if condition");

        auto thenBranch = statement();
        AST::StmtPtr elseBranch = nullptr;
        if (match(TokenType::ELSE))
        {
            elseBranch = statement();
        }

        return std::make_unique<AST::IfStmt>(std::move(condition), std::move(thenBranch),
                                             std::move(elseBranch), line);
    }

    AST::StmtPtr whileStatement()
    {
        int line = previous().line;
        consume(TokenType::LPAREN, "Expect '(' after 'while'");
        auto condition = expression();
        consume(TokenType::RPAREN, "Expect ')' after condition");
        auto body = statement();

        return std::make_unique<AST::WhileStmt>(std::move(condition), std::move(body), line);
    }

    AST::StmtPtr forStatement()
    {
        int line = previous().line;
        consume(TokenType::LPAREN, "Expect '(' after 'for'");

        AST::StmtPtr initializer;
        if (match(TokenType::SEMICOLON))
        {
            initializer = nullptr;
        }
        else if (match(TokenType::KEYWORD_VAR_NUM))
        {
            initializer = numVarDeclaration();
        }
        else
        {
            initializer = expressionStatement();
        }

        AST::ExprPtr condition = nullptr;
        if (!check(TokenType::SEMICOLON))
        {
            condition = expression();
        }
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition");

        AST::ExprPtr increment = nullptr;
        if (!check(TokenType::RPAREN))
        {
            increment = expression();
            while (!check(TokenType::RPAREN) && !isAtEnd())
            {
                if (match({TokenType::PLUS_PLUS, TokenType::MINUS_MINUS}))
                {
                    increment = std::make_unique<AST::UnaryExpr>(
                        previous().type,
                        std::move(increment),
                        previous().line);
                }
                else
                {
                    break;
                }
            }
        }
        consume(TokenType::RPAREN, "Expect ')' after for clauses");

        auto body = statement();

        AST::StmtPtr whileBody;
        if (increment)
        {
            std::vector<AST::StmtPtr> bodyStmts;
            bodyStmts.push_back(std::move(body));
            bodyStmts.push_back(std::make_unique<AST::ExprStmt>(std::move(increment), line));
            whileBody = std::make_unique<AST::BlockStmt>(std::move(bodyStmts), line);
        }
        else
        {
            whileBody = std::move(body);
        }

        auto whileLoop = std::make_unique<AST::WhileStmt>(
            condition ? std::move(condition)
                      : std::make_unique<AST::LiteralExpr>("1", TokenType::NUMBER, line),
            std::move(whileBody),
            line);

        if (initializer)
        {
            std::vector<AST::StmtPtr> stmts;
            stmts.push_back(std::move(initializer));
            stmts.push_back(std::move(whileLoop));
            return std::make_unique<AST::BlockStmt>(std::move(stmts), line);
        }

        return whileLoop;
    }

    AST::StmtPtr block()
    {
        int line = previous().line;
        std::vector<AST::StmtPtr> statements;

        while (!check(TokenType::RBRACE) && !isAtEnd())
        {
            statements.push_back(declaration());
        }

        consume(TokenType::RBRACE, "Expect '}' after block");
        return std::make_unique<AST::BlockStmt>(std::move(statements), line);
    }

    AST::StmtPtr returnStatement()
    {
        int line = previous().line;
        AST::ExprPtr value = nullptr;
        if (!check(TokenType::SEMICOLON))
        {
            value = expression();
        }

        consume(TokenType::SEMICOLON, "Expect ';' after return value");
        return std::make_unique<AST::ReturnStmt>(std::move(value), line);
    }

    AST::StmtPtr printStatement()
    {
        int line = previous().line;
        consume(TokenType::LPAREN, "Expect '(' after 'print'");
        auto value = expression();
        consume(TokenType::RPAREN, "Expect ')' after print expression");
        consume(TokenType::SEMICOLON, "Expect ';' after print statement");

        std::vector<AST::ExprPtr> args;
        args.push_back(std::move(value));

        auto call = std::make_unique<AST::CallExpr>("print", std::move(args), line);
        return std::make_unique<AST::ExprStmt>(std::move(call), line);
    }

    AST::StmtPtr expressionStatement()
    {
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression");
        return std::make_unique<AST::ExprStmt>(std::move(expr), previous().line);
    }

    AST::ExprPtr expression()
    {
        return assignment();
    }

    AST::ExprPtr assignment()
    {
        auto expr = logicalOr();

        if (match({TokenType::ASSIGN, TokenType::PLUS_EQ, TokenType::MINUS_EQ,
                   TokenType::ASTER_EQ, TokenType::FSLASH_EQ, TokenType::PERCENT_EQ}))
        {
            TokenType op = previous().type;
            auto value = assignment();
            if (dynamic_cast<AST::IdentifierExpr *>(expr.get()))
            {
                return std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(value), previous().line);
            }

            throw parseError(previous(), "Invalid assignment target");
        }

        return expr;
    }

    AST::ExprPtr logicalOr()
    {
        auto expr = logicalAnd();

        while (match(TokenType::LOGICAL_OR))
        {
            TokenType op = previous().type;
            auto right = logicalAnd();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr logicalAnd()
    {
        auto expr = bitwiseOr();

        while (match(TokenType::LOGICAL_AND))
        {
            TokenType op = previous().type;
            auto right = bitwiseOr();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr bitwiseOr()
    {
        auto expr = bitwiseXor();

        while (match(TokenType::OR))
        {
            TokenType op = previous().type;
            auto right = bitwiseXor();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr bitwiseXor()
    {
        auto expr = bitwiseAnd();

        while (match(TokenType::XOR))
        {
            TokenType op = previous().type;
            auto right = bitwiseAnd();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr bitwiseAnd()
    {
        auto expr = equality();

        while (match(TokenType::AND))
        {
            TokenType op = previous().type;
            auto right = equality();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr equality()
    {
        auto expr = comparison();

        while (match({TokenType::EQ_EQ, TokenType::BANG_EQ}))
        {
            TokenType op = previous().type;
            auto right = comparison();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr comparison()
    {
        auto expr = term();

        while (match({TokenType::LT, TokenType::GT}))
        {
            TokenType op = previous().type;
            auto right = term();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr term()
    {
        auto expr = factor();

        while (match({TokenType::PLUS, TokenType::MINUS}))
        {
            TokenType op = previous().type;
            auto right = factor();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr factor()
    {
        auto expr = unary();

        while (match({TokenType::ASTER, TokenType::FSLASH, TokenType::PERCENT}))
        {
            TokenType op = previous().type;
            auto right = unary();
            expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right), previous().line);
        }

        return expr;
    }

    AST::ExprPtr unary()
    {
        if (match({TokenType::NOT, TokenType::MINUS, TokenType::PLUS_PLUS, TokenType::MINUS_MINUS}))
        {
            TokenType op = previous().type;
            auto right = unary();
            return std::make_unique<AST::UnaryExpr>(op, std::move(right), previous().line);
        }

        return call();
    }

    AST::ExprPtr call()
    {
        auto expr = primary();

        while (true)
        {
            if (match(TokenType::LPAREN))
            {
                expr = finishCall(std::move(expr));
            }
            else
            {
                break;
            }
        }

        return expr;
    }

    AST::ExprPtr finishCall(AST::ExprPtr callee)
    {
        std::vector<AST::ExprPtr> arguments;
        int line = previous().line;

        if (!check(TokenType::RPAREN))
        {
            do
            {
                arguments.push_back(expression());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RPAREN, "Expect ')' after arguments");

        if (auto ident = dynamic_cast<AST::IdentifierExpr *>(callee.get()))
        {
            return std::make_unique<AST::CallExpr>(
                ident->name,
                std::move(arguments),
                line);
        }

        throw parseError(previous(), "Can only call functions");
    }

    AST::ExprPtr primary()
    {
        if (match(TokenType::NUMBER) || match(TokenType::STRING_LIT))
        {
            return std::make_unique<AST::LiteralExpr>(previous().value.value(), previous().type, previous().line);
        }

        if (match(TokenType::IDENTIFIER))
        {
            return std::make_unique<AST::IdentifierExpr>(previous().value.value(), previous().line);
        }

        if (match(TokenType::LPAREN))
        {
            auto expr = expression();
            consume(TokenType::RPAREN, "Error expected ')' after expression");
            return expr;
        }

        throw parseError(peek(), "Error expected expression");
    }
};