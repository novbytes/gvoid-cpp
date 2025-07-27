#pragma once

#include <memory>
#include <vector>

#include "tokens.hpp"

namespace AST
{

    struct Expr;
    struct Stmt;

    using ExprPtr = std::unique_ptr<Expr>;
    using StmtPtr = std::unique_ptr<Stmt>;
    using StmtList = std::vector<StmtPtr>;

    struct Expr
    {
        virtual ~Expr() = default;
        int line;
        explicit Expr(int line) : line(line) {}
    };

    struct BinaryExpr : Expr
    {
        ExprPtr left;
        TokenType op;
        ExprPtr right;

        BinaryExpr(ExprPtr left, TokenType op, ExprPtr right, int line)
            : Expr(line), left(std::move(left)), op(op), right(std::move(right)) {}
    };

    struct UnaryExpr : Expr
    {
        TokenType op;
        ExprPtr right;

        UnaryExpr(TokenType op, ExprPtr right, int line)
            : Expr(line), op(op), right(std::move(right)) {}
    };

    struct LiteralExpr : Expr
    {
        std::string value;
        TokenType type;

        LiteralExpr(const std::string &value, TokenType type, int line)
            : Expr(line), value(value), type(type) {}
    };

    struct IdentifierExpr : Expr
    {
        std::string name;

        explicit IdentifierExpr(const std::string &name, int line)
            : Expr(line), name(name) {}
    };

    struct CallExpr : Expr
    {
        std::string callee;
        std::vector<ExprPtr> args;

        CallExpr(std::string callee, std::vector<ExprPtr> &&args, int line)
            : Expr(line), callee(std::move(callee)), args(std::move(args)) {}
    };

    struct Stmt
    {
        virtual ~Stmt() = default;
        int line;
        explicit Stmt(int line) : line(line) {}
    };

    struct ImportStmt : public Stmt
    {
        std::string moduleName;

        ImportStmt(std::string moduleName, int line)
            : Stmt(line), // Base class first
              moduleName(std::move(moduleName)) {} 
    };

    struct VarDeclStmt : public Stmt
    {
        std::string type;
        std::string name;
        std::unique_ptr<Expr> initializer;

        VarDeclStmt(std::string type, std::string name,
                    std::unique_ptr<Expr> initializer, int line)
            : Stmt(line),            
              type(std::move(type)), 
              name(std::move(name)),
              initializer(std::move(initializer))
        {
        }
    };

    struct ExprStmt : Stmt
    {
        ExprPtr expr;

        explicit ExprStmt(ExprPtr expr, int line)
            : Stmt(line), expr(std::move(expr)) {}
    };

    struct BlockStmt : Stmt
    {
        StmtList statements;

        explicit BlockStmt(StmtList statements, int line)
            : Stmt(line), statements(std::move(statements)) {}
    };

    struct IfStmt : Stmt
    {
        ExprPtr condition;
        StmtPtr thenBranch;
        StmtPtr elseBranch;

        IfStmt(ExprPtr condition, StmtPtr thenBranch, StmtPtr elseBranch, int line)
            : Stmt(line), condition(std::move(condition)),
              thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
    };

    struct ForStmt : Stmt
    {
        StmtPtr initializer;
        ExprPtr condition;
        ExprPtr increment;
        StmtPtr body;

        ForStmt(StmtPtr initializer, ExprPtr condition, ExprPtr increment, StmtPtr body, int line)
            : Stmt(line), initializer(std::move(initializer)),
              condition(std::move(condition)),
              increment(std::move(increment)),
              body(std::move(body)) {}
    };

    struct WhileStmt : Stmt
    {
        ExprPtr condition;
        StmtPtr body;

        WhileStmt(ExprPtr condition, StmtPtr body, int line)
            : Stmt(line), condition(std::move(condition)), body(std::move(body)) {}
    };

    struct FunctionStmt : Stmt
    {
        std::string name;
        std::vector<std::string> params;
        StmtPtr body;

        FunctionStmt(const std::string &name, std::vector<std::string> params, StmtPtr body, int line)
            : Stmt(line), name(name), params(std::move(params)), body(std::move(body)) {}
    };

    struct ReturnStmt : Stmt
    {
        ExprPtr value;

        explicit ReturnStmt(ExprPtr value, int line)
            : Stmt(line), value(std::move(value)) {}
    };

}