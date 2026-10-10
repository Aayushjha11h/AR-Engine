#pragma once
#include <string>
#include <cstdint>
#include <cmath>

namespace ar {

    enum class ValType {
        Nil,
        Int,
        Float,
        Bool,
        String
    };

    struct Value {
        ValType type = ValType::Nil;
        int64_t iVal = 0;
        double  fVal = 0.0;
        bool    bVal = false;
        std::string sVal;

        Value() : type(ValType::Nil) {}

        static Value Nil() {
            return Value();
        }

        static Value Int(int64_t val) {
            Value v;
            v.type = ValType::Int;
            v.iVal = val;
            v.fVal = static_cast<double>(val);
            return v;
        }

        static Value Float(double val) {
            Value v;
            v.type = ValType::Float;
            v.fVal = val;
            v.iVal = static_cast<int64_t>(val);
            return v;
        }

        static Value Bool(bool val) {
            Value v;
            v.type = ValType::Bool;
            v.bVal = val;
            return v;
        }

        static Value Str(const std::string& val) {
            Value v;
            v.type = ValType::String;
            v.sVal = val;
            return v;
        }

        bool AsBool() const;
        double AsNumber() const;
        int64_t AsInt() const;
        std::string AsString() const;
    };

    Value operator+(const Value& lhs, const Value& rhs);
    Value operator-(const Value& lhs, const Value& rhs);
    Value operator*(const Value& lhs, const Value& rhs);
    Value operator/(const Value& lhs, const Value& rhs);
    Value operator%(const Value& lhs, const Value& rhs);

    bool ValueEqual(const Value& lhs, const Value& rhs);
    bool ValueLess(const Value& lhs, const Value& rhs);

    inline bool operator==(const Value& lhs, const Value& rhs) { return ValueEqual(lhs, rhs); }
    inline bool operator!=(const Value& lhs, const Value& rhs) { return !ValueEqual(lhs, rhs); }
    inline bool operator<(const Value& lhs, const Value& rhs) { return ValueLess(lhs, rhs); }
    inline bool operator<=(const Value& lhs, const Value& rhs) { return ValueLess(lhs, rhs) || ValueEqual(lhs, rhs); }
    inline bool operator>(const Value& lhs, const Value& rhs) { return !ValueLess(lhs, rhs) && !ValueEqual(lhs, rhs); }
    inline bool operator>=(const Value& lhs, const Value& rhs) { return !ValueLess(lhs, rhs); }

} // namespace ar
