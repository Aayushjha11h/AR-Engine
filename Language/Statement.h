#pragma once
#include "Expr.h"
#include <string>
#include <vector>
#include <memory>

namespace ar {

    enum class StatementKind {
        Command,
        Set,
        If,
        While,
        For,
        Block
    };

    struct Statement {
        StatementKind kind = StatementKind::Command;

        // Command: holds a vector of raw tokens
        std::vector<std::string> tokens;

        // Set: variable name + Expr
        std::string varName;
        std::unique_ptr<Expr> valueExpr;

        // If / While: condition
        std::unique_ptr<Expr> condition;

        // For: loop variable name + start Expr + end Expr
        std::unique_ptr<Expr> startExpr;
        std::unique_ptr<Expr> endExpr;

        // Bodies
        std::vector<Statement> body;
        std::vector<Statement> elseBody;

        Statement() = default;
        ~Statement() = default;

        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;
        Statement(Statement&&) noexcept = default;
        Statement& operator=(Statement&&) noexcept = default;

        static Statement MakeCommand(std::vector<std::string> toks) {
            Statement s;
            s.kind = StatementKind::Command;
            s.tokens = std::move(toks);
            return s;
        }

        static Statement MakeSet(const std::string& var, std::unique_ptr<Expr> val) {
            Statement s;
            s.kind = StatementKind::Set;
            s.varName = var;
            s.valueExpr = std::move(val);
            return s;
        }

        static Statement MakeIf(std::unique_ptr<Expr> cond, std::vector<Statement> b, std::vector<Statement> elseB = {}) {
            Statement s;
            s.kind = StatementKind::If;
            s.condition = std::move(cond);
            s.body = std::move(b);
            s.elseBody = std::move(elseB);
            return s;
        }

        static Statement MakeWhile(std::unique_ptr<Expr> cond, std::vector<Statement> b) {
            Statement s;
            s.kind = StatementKind::While;
            s.condition = std::move(cond);
            s.body = std::move(b);
            return s;
        }

        static Statement MakeFor(const std::string& var, std::unique_ptr<Expr> st, std::unique_ptr<Expr> en, std::vector<Statement> b) {
            Statement s;
            s.kind = StatementKind::For;
            s.varName = var;
            s.startExpr = std::move(st);
            s.endExpr = std::move(en);
            s.body = std::move(b);
            return s;
        }

        static Statement MakeBlock(std::vector<Statement> b) {
            Statement s;
            s.kind = StatementKind::Block;
            s.body = std::move(b);
            return s;
        }
    };

} // namespace ar
