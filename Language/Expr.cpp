#include "Expr.h"
#include "../engine/Entity.h"
#include "../engine/Component.h"
#include "../engine/Health.h"
#include "../engine/RigidBody.h"
#include "RuntimeBridge.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace ar {

    std::unique_ptr<Expr> Expr::MakeLiteral(const Value& val) {
        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Literal;
        expr->literalValue = val;
        return expr;
    }

    std::unique_ptr<Expr> Expr::MakeVar(const std::string& name) {
        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Var;
        expr->varName = name;
        return expr;
    }

    std::unique_ptr<Expr> Expr::MakeMember(const std::string& ent, const std::string& member) {
        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Member;
        expr->entityName = ent;
        expr->memberName = member;
        return expr;
    }

    std::unique_ptr<Expr> Expr::MakeCall(const std::string& func, std::vector<std::unique_ptr<Expr>> arguments) {
        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Call;
        expr->funcName = func;
        expr->args = std::move(arguments);
        return expr;
    }

    std::unique_ptr<Expr> Expr::MakeBinary(const std::string& oper, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) {
        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Binary;
        expr->op = oper;
        expr->left = std::move(lhs);
        expr->right = std::move(rhs);
        return expr;
    }

    std::unique_ptr<Expr> Expr::MakeUnary(const std::string& oper, std::unique_ptr<Expr> opnd) {
        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Unary;
        expr->op = oper;
        expr->operand = std::move(opnd);
        return expr;
    }

    Value Expr::Eval(const Context& ctx) const {
        switch (kind) {
        case ExprKind::Literal:
            return literalValue;

        case ExprKind::Var: {
            if (ctx.entity) {
                auto it = ctx.entity->Variables.find(varName);
                if (it != ctx.entity->Variables.end()) {
                    return it->second;
                }
            }
            return Value::Nil();
        }

        case ExprKind::Member: {
            Entity* target = nullptr;
            if (entityName == "self") {
                target = ctx.entity;
            }
            else if (ctx.bridge) {
                target = ctx.bridge->GetEntity(entityName);
            }
            if (!target) return Value::Nil();

            if (memberName == "x") {
                auto* t = target->GetComponent<Transform>();
                return t ? Value::Float(t->Position.x) : Value::Nil();
            }
            if (memberName == "y") {
                auto* t = target->GetComponent<Transform>();
                return t ? Value::Float(t->Position.y) : Value::Nil();
            }
            if (memberName == "health") {
                auto* h = target->GetComponent<Health>();
                return h ? Value::Int(h->Current) : Value::Nil();
            }
            if (memberName == "max_health") {
                auto* h = target->GetComponent<Health>();
                return h ? Value::Int(h->Max) : Value::Nil();
            }
            if (memberName == "vx") {
                auto* rb = target->GetComponent<RigidBody>();
                return rb ? Value::Float(rb->Velocity.x) : Value::Nil();
            }
            if (memberName == "vy") {
                auto* rb = target->GetComponent<RigidBody>();
                return rb ? Value::Float(rb->Velocity.y) : Value::Nil();
            }

            auto it = target->Variables.find(memberName);
            if (it != target->Variables.end()) {
                return it->second;
            }
            return Value::Nil();
        }

        case ExprKind::Call: {
            if (funcName == "exists") {
                if (args.empty()) return Value::Bool(false);
                std::string targetName = args[0]->Eval(ctx).AsString();
                Entity* e = ctx.bridge ? ctx.bridge->GetEntity(targetName) : nullptr;
                return Value::Bool(e != nullptr);
            }

            if (funcName == "distance_to") {
                if (args.empty()) return Value::Float(0.0);
                std::string targetName = args[0]->Eval(ctx).AsString();
                Entity* other = ctx.bridge ? ctx.bridge->GetEntity(targetName) : nullptr;
                if (ctx.entity && other) {
                    auto* t1 = ctx.entity->GetComponent<Transform>();
                    auto* t2 = other->GetComponent<Transform>();
                    if (t1 && t2) {
                        float dx = t2->Position.x - t1->Position.x;
                        float dy = t2->Position.y - t1->Position.y;
                        return Value::Float(std::sqrt(dx * dx + dy * dy));
                    }
                }
                return Value::Float(0.0);
            }

            if (funcName == "random") {
                int64_t lo = args.size() > 0 ? args[0]->Eval(ctx).AsInt() : 0;
                int64_t hi = args.size() > 1 ? args[1]->Eval(ctx).AsInt() : 0;
                if (lo > hi) std::swap(lo, hi);
                int64_t diff = hi - lo + 1;
                int64_t res = diff > 0 ? (lo + (std::rand() % diff)) : lo;
                return Value::Int(res);
            }

            return Value::Nil();
        }

        case ExprKind::Binary: {
            Value l = left ? left->Eval(ctx) : Value::Nil();
            Value r = right ? right->Eval(ctx) : Value::Nil();

            if (op == "+") return l + r;
            if (op == "-") return l - r;
            if (op == "*") return l * r;
            if (op == "/") return l / r;
            if (op == "%") return l % r;

            if (op == "==") return Value::Bool(ValueEqual(l, r));
            if (op == "!=") return Value::Bool(!ValueEqual(l, r));
            if (op == "<")  return Value::Bool(ValueLess(l, r));
            if (op == "<=") return Value::Bool(ValueLess(l, r) || ValueEqual(l, r));
            if (op == ">")  return Value::Bool(!ValueLess(l, r) && !ValueEqual(l, r));
            if (op == ">=") return Value::Bool(!ValueLess(l, r));

            if (op == "and") return Value::Bool(l.AsBool() && r.AsBool());
            if (op == "or")  return Value::Bool(l.AsBool() || r.AsBool());

            return Value::Nil();
        }

        case ExprKind::Unary: {
            Value val = operand ? operand->Eval(ctx) : Value::Nil();
            if (op == "-") {
                if (val.type == ValType::Float) return Value::Float(-val.fVal);
                return Value::Int(-val.AsInt());
            }
            if (op == "!" || op == "not") {
                return Value::Bool(!val.AsBool());
            }
            return Value::Nil();
        }

        default:
            return Value::Nil();
        }
    }

} // namespace ar