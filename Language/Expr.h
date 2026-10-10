#pragma once
#include "Value.h"
#include <string>
#include <vector>
#include <memory>

namespace ar {

    class Entity;
    class RuntimeBridge;

    enum class ExprKind {
        Literal,
        Var,
        Member,
        Call,
        Binary,
        Unary
    };

    struct Expr {
        struct Context {
            Entity* entity = nullptr;
            RuntimeBridge* bridge = nullptr;
        };

        ExprKind kind = ExprKind::Literal;

        Value literalValue;                           // for Literal
        std::string varName;                          // for Var ($x)
        std::string entityName;                       // for Member (Player.x)
        std::string memberName;                       // for Member
        std::string funcName;                         // for Call
        std::vector<std::unique_ptr<Expr>> args;      // for Call
        std::string op;                               // for Binary / Unary
        std::unique_ptr<Expr> left;                   // for Binary
        std::unique_ptr<Expr> right;                  // for Binary
        std::unique_ptr<Expr> operand;                // for Unary

        Expr() = default;
        ~Expr() = default;

        Expr(const Expr&) = delete;
        Expr& operator=(const Expr&) = delete;
        Expr(Expr&&) noexcept = default;
        Expr& operator=(Expr&&) noexcept = default;

        static std::unique_ptr<Expr> MakeLiteral(const Value& val);
        static std::unique_ptr<Expr> MakeVar(const std::string& name);
        static std::unique_ptr<Expr> MakeMember(const std::string& ent, const std::string& member);
        static std::unique_ptr<Expr> MakeCall(const std::string& func, std::vector<std::unique_ptr<Expr>> arguments);
        static std::unique_ptr<Expr> MakeBinary(const std::string& oper, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs);
        static std::unique_ptr<Expr> MakeUnary(const std::string& oper, std::unique_ptr<Expr> opnd);

        Value Eval(const Context& ctx) const;
    };

} // namespace ar
