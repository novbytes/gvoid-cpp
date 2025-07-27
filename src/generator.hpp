#pragma once

#include "ast.hpp"
#include "parser.hpp"
#include "tokens.hpp"
#include <sstream>
#include <unordered_map>
#include <memory>
#include <algorithm>

class Generator
{
public:
    explicit Generator(const AST::StmtList &statements)
        : m_statements(statements) {}

    std::string generate()
    {
        std::stringstream ss;

        ss << "#include <iostream>\n";
        ss << "#include <vector>\n";
        ss << "#include <string>\n";
        ss << "#include <unordered_map>\n";
        ss << "#include <cmath>\n\n";
        ss << "using namespace std;\n\n";

        for (const auto &stmt : m_statements)
        {
            if (auto varDecl = dynamic_cast<const AST::VarDeclStmt *>(stmt.get()))
            {
                ss << mapType(varDecl->type) << " " << varDecl->name;
                if (varDecl->initializer)
                {
                    ss << " = ";
                    generateExpr(*varDecl->initializer, ss);
                }
                ss << ";\n";
            }
            else if (auto func = dynamic_cast<const AST::FunctionStmt *>(stmt.get()))
            {
                ss << m_functionReturnTypes[func->name] << " " << func->name << "(";
                ss << ");\n";
            }
        }

        ss << "int main() {\n";

        for (const auto &stmt : m_statements)
        {
            if (dynamic_cast<const AST::VarDeclStmt *>(stmt.get()))
            {
            }
            else if (dynamic_cast<const AST::FunctionStmt *>(stmt.get()))
            {
            }
            else
            {
                generateStatement(*stmt, ss);
            }
        }

        ss << "    return 0;\n";
        ss << "}\n";

        for (const auto &stmt : m_statements)
        {
            if (auto func = dynamic_cast<const AST::FunctionStmt *>(stmt.get()))
            {
                generateFunction(*func, ss);
            }
        }

        return ss.str();
    }

private:
    bool hasMainFunction = false;
    const AST::StmtList &m_statements;
    std::unordered_map<std::string, std::string> m_varTypes;
    std::unordered_map<std::string, std::string> m_functionReturnTypes;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> m_functionParams;

    void generateForwardDeclarations(std::stringstream &ss)
    {
        for (const auto &stmt : m_statements)
        {
            if (auto func = dynamic_cast<const AST::FunctionStmt *>(stmt.get()))
            {
                if (func->name == "main")
                {
                    hasMainFunction = true;
                }
                std::string returnType = inferFunctionReturnType(*func);
                m_functionReturnTypes[func->name] = returnType;

                std::vector<std::pair<std::string, std::string>> params;
                for (const auto &param : func->params)
                {
                    params.emplace_back("int", param);
                }
                m_functionParams[func->name] = params;

                ss << returnType << " " << func->name << "(";
                for (size_t i = 0; i < func->params.size(); ++i)
                {
                    ss << "int " << func->params[i];
                    if (i != func->params.size() - 1)
                    {
                        ss << ", ";
                    }
                }
                ss << ");\n";
            }
        }
        ss << "\n";
    }

    std::string inferFunctionReturnType(const AST::FunctionStmt &func)
    {
        std::string returnType = "void";

        if (auto block = dynamic_cast<const AST::BlockStmt *>(func.body.get()))
        {
            for (const auto &stmt : block->statements)
            {
                if (auto ret = dynamic_cast<const AST::ReturnStmt *>(stmt.get()))
                {
                    if (ret->value)
                    {
                        returnType = inferExprType(*ret->value);
                    }
                    break;
                }
            }
        }

        return returnType;
    }

    std::string inferExprType(const AST::Expr &expr)
    {
        if (auto literal = dynamic_cast<const AST::LiteralExpr *>(&expr))
        {
            if (literal->type == TokenType::STRING_LIT)
                return "std::string";
            if (literal->type == TokenType::NUMBER)
                return "double";
        }
        else if (auto ident = dynamic_cast<const AST::IdentifierExpr *>(&expr))
        {
            if (m_varTypes.count(ident->name))
                return m_varTypes[ident->name];
        }
        else if (auto call = dynamic_cast<const AST::CallExpr *>(&expr))
        {
            if (m_functionReturnTypes.count(call->callee))
            {
                return m_functionReturnTypes[call->callee];
            }
        }

        return "int";
    }

    void generateStatement(const AST::Stmt &stmt, std::stringstream &ss)
    {
        if (auto import = dynamic_cast<const AST::ImportStmt *>(&stmt))
        {
            generateImport(*import, ss);
        }
        else if (auto varDecl = dynamic_cast<const AST::VarDeclStmt *>(&stmt))
        {
            generateVarDecl(*varDecl, ss);
        }
        else if (auto func = dynamic_cast<const AST::FunctionStmt *>(&stmt))
        {
            generateFunction(*func, ss);
        }
        else if (auto expr = dynamic_cast<const AST::ExprStmt *>(&stmt))
        {
            generateExpr(*expr->expr, ss);
            ss << ";\n";
        }
        else if (auto block = dynamic_cast<const AST::BlockStmt *>(&stmt))
        {
            ss << "{\n";
            for (const auto &s : block->statements)
            {
                generateStatement(*s, ss);
            }
            ss << "}\n";
        }
        else if (auto ifStmt = dynamic_cast<const AST::IfStmt *>(&stmt))
        {
            generateIf(*ifStmt, ss);
        }
        else if (auto forStmt = dynamic_cast<const AST::ForStmt *>(&stmt))
        {
            generateFor(*forStmt, ss);
        }
        else if (auto whileStmt = dynamic_cast<const AST::WhileStmt *>(&stmt))
        {
            generateWhile(*whileStmt, ss);
        }
        else if (auto ret = dynamic_cast<const AST::ReturnStmt *>(&stmt))
        {
            ss << "return ";
            if (ret->value)
                generateExpr(*ret->value, ss);
            ss << ";\n";
        }
    }

    void generateImport(const AST::ImportStmt &import, std::stringstream &ss)
    {
        static const std::unordered_map<std::string, std::string> importMap = {
            {"io", "<iostream>"},
            {"math", "<cmath>"},
            {"vector", "<vector>"},
            {"string", "<string>"},
            {"map", "<unordered_map>"}};

        auto it = importMap.find(import.moduleName);
        if (it != importMap.end())
        {
            ss << "#include " << it->second << "\n";
        }
        else
        {
            ss << "// (Import state is coming soon) Import: " << import.moduleName << "\n";
        }
    }

    void generateVarDecl(const AST::VarDeclStmt &varDecl, std::stringstream &ss)
    {
        std::string cppType = mapType(varDecl.type);
        ss << cppType << " " << varDecl.name;

        if (varDecl.initializer)
        {
            ss << " = ";
            generateExpr(*varDecl.initializer, ss);
        }
        ss << ";\n";
        m_varTypes[varDecl.name] = cppType;
    }

    std::string mapType(const std::string &type)
    {
        static const std::unordered_map<std::string, std::string> typeMap = {
            {"num", "double"},
            {"str", "std::string"},
            {"arr", "std::vector<double>"},
            {"bool", "bool"},
            {"void", "void"}};

        auto it = typeMap.find(type);
        if (it != typeMap.end())
        {
            return it->second;
        }
        return type;
    }

    void generateFunction(const AST::FunctionStmt &func, std::stringstream &ss)
    {
        std::string returnType = m_functionReturnTypes[func.name];

        ss << returnType << " " << func.name << "(";

        const auto &params = m_functionParams[func.name];
        for (size_t i = 0; i < func.params.size(); ++i)
        {
            ss << params[i].first << " " << func.params[i];
            if (i != func.params.size() - 1)
            {
                ss << ", ";
            }
        }

        ss << ") ";
        generateStatement(*func.body, ss);
        ss << "\n";
    }

    void generateIf(const AST::IfStmt &ifStmt, std::stringstream &ss)
    {
        ss << "if (";
        generateExpr(*ifStmt.condition, ss);
        ss << ") ";
        generateStatement(*ifStmt.thenBranch, ss);

        if (ifStmt.elseBranch)
        {
            ss << "else ";
            generateStatement(*ifStmt.elseBranch, ss);
        }
    }

    void generateFor(const AST::ForStmt &forStmt, std::stringstream &ss)
    {
        ss << "for (";
        if (forStmt.initializer)
        {
            generateStatement(*forStmt.initializer, ss);
            std::string code = ss.str();
            if (!code.empty() && code.back() == '\n')
            {
                ss.seekp(-1, std::ios_base::end);
            }
        }
        else
        {
            ss << ";";
        }

        ss << " ";
        if (forStmt.condition)
        {
            generateExpr(*forStmt.condition, ss);
        }
        ss << "; ";

        if (forStmt.increment)
        {
            generateExpr(*forStmt.increment, ss);
        }
        ss << ") ";

        generateStatement(*forStmt.body, ss);
    }

    void generateWhile(const AST::WhileStmt &whileStmt, std::stringstream &ss)
    {
        ss << "while (";
        generateExpr(*whileStmt.condition, ss);
        ss << ") ";
        generateStatement(*whileStmt.body, ss);
    }

    void generateExpr(const AST::Expr &expr, std::stringstream &ss)
    {
        if (auto binary = dynamic_cast<const AST::BinaryExpr *>(&expr))
        {
            generateBinaryExpr(*binary, ss);
        }
        else if (auto unary = dynamic_cast<const AST::UnaryExpr *>(&expr))
        {
            generateUnaryExpr(*unary, ss);
        }
        else if (auto literal = dynamic_cast<const AST::LiteralExpr *>(&expr))
        {
            generateLiteral(*literal, ss);
        }
        else if (auto ident = dynamic_cast<const AST::IdentifierExpr *>(&expr))
        {
            ss << ident->name;
        }
        else if (auto call = dynamic_cast<const AST::CallExpr *>(&expr))
        {
            if (call->callee == "print")
            {
                generatePrintCall(*call, ss);
            }
            else
            {
                ss << call->callee << "(";
                for (size_t i = 0; i < call->args.size(); ++i)
                {
                    generateExpr(*call->args[i], ss);
                    if (i != call->args.size() - 1)
                    {
                        ss << ", ";
                    }
                }
                ss << ")";
            }
        }
        else if (auto call = dynamic_cast<const AST::CallExpr *>(&expr))
        {
            generateCall(*call, ss);
        }
    }

    void generateBinaryExpr(const AST::BinaryExpr &expr, std::stringstream &ss)
    {
        if (expr.op == TokenType::STREAM_OUT)
        {
            ss << "std::cout << ";
            generateExpr(*expr.right, ss);
            return;
        }

        ss << "(";
        generateExpr(*expr.left, ss);

        switch (expr.op)
        {
        case TokenType::PLUS:
            ss << " + ";
            break;
        case TokenType::MINUS:
            ss << " - ";
            break;
        case TokenType::ASTER:
            ss << " * ";
            break;
        case TokenType::FSLASH:
            ss << " / ";
            break;
        case TokenType::PERCENT:
            ss << " % ";
            break;

        // Compound assignment
        case TokenType::PLUS_EQ:
            ss << " += ";
            break;
        case TokenType::MINUS_EQ:
            ss << " -= ";
            break;
        case TokenType::ASTER_EQ:
            ss << " *= ";
            break;
        case TokenType::FSLASH_EQ:
            ss << " /= ";
            break;
        case TokenType::PERCENT_EQ:
            ss << " %= ";
            break;

        // Comparison
        case TokenType::EQ_EQ:
            ss << " == ";
            break;
        case TokenType::BANG_EQ:
            ss << " != ";
            break;
        case TokenType::LT:
            ss << " < ";
            break;
        case TokenType::GT:
            ss << " > ";
            break;
        case TokenType::LT_EQ:
            ss << " <= ";
            break;
        case TokenType::GT_EQ:
            ss << " >= ";
            break;

        // Logical
        case TokenType::LOGICAL_AND:
            ss << " && ";
            break;
        case TokenType::LOGICAL_OR:
            ss << " || ";
            break;

        // Bitwise
        case TokenType::AND:
            ss << " & ";
            break;
        case TokenType::OR:
            ss << " | ";
            break;
        case TokenType::XOR:
            ss << " ^ ";
            break;

        // Other
        case TokenType::ARROW_RIGHT:
            ss << "->";
            break;

        default:
            ss << " " << tokenTypeToString(expr.op) << " ";
            break;
        }

        generateExpr(*expr.right, ss);
        ss << ")";
    }

    void generateUnaryExpr(const AST::UnaryExpr &expr, std::stringstream &ss)
    {
        switch (expr.op)
        {
        case TokenType::MINUS:
            ss << "-";
            break;
        case TokenType::NOT:
            ss << "!";
            break;
        case TokenType::PLUS_PLUS:
            ss << "++";
            break;
        case TokenType::MINUS_MINUS:
            ss << "--";
            break;
        case TokenType::BITWISE_NOT:
            ss << "~";
            break;
        default:
            ss << tokenTypeToString(expr.op);
            break;
        }
        generateExpr(*expr.right, ss);
    }

    void generateLiteral(const AST::LiteralExpr &literal, std::stringstream &ss)
    {
        switch (literal.type)
        {
        case TokenType::STRING_LIT:
            ss << "\"" << escapeString(literal.value) << "\"";
            break;
        case TokenType::NUMBER:
            ss << literal.value;
            break;
        case TokenType::TRUE:
            ss << "true";
            break;
        case TokenType::FALSE:
            ss << "false";
            break;
        default:
            ss << literal.value;
        }
    }

    std::string escapeString(const std::string &str)
    {
        std::string result;
        for (char c : str)
        {
            switch (c)
            {
            case '\n':
                result += "\\n";
                break;
            case '\t':
                result += "\\t";
                break;
            case '\"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            default:
                result += c;
                break;
            }
        }
        return result;
    }

    void generateCall(const AST::CallExpr &call, std::stringstream &ss)
    {
        if (call.callee == "print")
        {
            generatePrintCall(call, ss);
        }
        else if (call.callee == "size")
        {
            if (!call.args.empty())
            {
                ss << "(";
                generateExpr(*call.args[0], ss);
                ss << ").size()";
            }
            else
            {
                ss << "0 /* size() called with no arguments */";
            }
        }
        else
        {
            ss << call.callee << "(";
            for (size_t i = 0; i < call.args.size(); ++i)
            {
                generateExpr(*call.args[i], ss);
                if (i != call.args.size() - 1)
                {
                    ss << ", ";
                }
            }
            ss << ")";
        }
    }

    void generatePrintCall(const AST::CallExpr &call, std::stringstream &ss)
    {
        ss << "cout";
        for (const auto &arg : call.args)
        {
            ss << " << ";
            generateExpr(*arg, ss);
        }
        ss << " << std::endl";
    }

    std::string tokenTypeToString(TokenType type)
    {
        static const std::unordered_map<TokenType, std::string> tokenMap = {
            {TokenType::PLUS, "+"},
            {TokenType::MINUS, "-"},
            {TokenType::ASTER, "*"},
            {TokenType::FSLASH, "/"},
            {TokenType::PERCENT, "%"},
            {TokenType::PLUS_EQ, "+="},
            {TokenType::MINUS_EQ, "-="},
            {TokenType::ASTER_EQ, "*="},
            {TokenType::FSLASH_EQ, "/="},
            {TokenType::PERCENT_EQ, "%="},
            {TokenType::EQ_EQ, "=="},
            {TokenType::BANG_EQ, "!="},
            {TokenType::LT, "<"},
            {TokenType::GT, ">"},
            {TokenType::LT_EQ, "<="},
            {TokenType::GT_EQ, ">="},
            {TokenType::LOGICAL_AND, "&&"},
            {TokenType::LOGICAL_OR, "||"},
            {TokenType::AND, "&"},
            {TokenType::OR, "|"},
            {TokenType::XOR, "^"},
            {TokenType::NOT, "!"},
            {TokenType::BITWISE_NOT, "~"},
            {TokenType::PLUS_PLUS, "++"},
            {TokenType::MINUS_MINUS, "--"},
            {TokenType::ARROW_RIGHT, "->"},
            {TokenType::ARROW_LEFT, "<-"},
            {TokenType::STREAM_OUT, "<<"},
            {TokenType::STREAM_IN, ">>"}};

        auto it = tokenMap.find(type);
        if (it != tokenMap.end())
        {
            return it->second;
        }
        return "/* unknown op */";
    }
};