#include "Value.h"
#include <sstream>
#include <iomanip>
#include <cmath>   

namespace ar {

    bool Value::AsBool() const {
        switch (type) {
        case ValType::Nil:
            return false;
        case ValType::Bool:
            return bVal;
        case ValType::Int:
            return iVal != 0;
        case ValType::Float:
            return std::abs(fVal) > 1e-6;
        case ValType::String:
            if (sVal.empty() || sVal == "false" || sVal == "0") return false;
            return true;
        default:
            return false;
        }
    }

    double Value::AsNumber() const {
        switch (type) {
        case ValType::Nil:
            return 0.0;
        case ValType::Bool:
            return bVal ? 1.0 : 0.0;
        case ValType::Int:
            return static_cast<double>(iVal);
        case ValType::Float:
            return fVal;
        case ValType::String:
            try {
                size_t idx = 0;
                double val = std::stod(sVal, &idx);
                return val;
            }
            catch (...) {
                return 0.0;
            }
        default:
            return 0.0;
        }
    }

    int64_t Value::AsInt() const {
        switch (type) {
        case ValType::Nil:
            return 0;
        case ValType::Bool:
            return bVal ? 1 : 0;
        case ValType::Int:
            return iVal;
        case ValType::Float:
            return static_cast<int64_t>(fVal);
        case ValType::String:
            try {
                size_t idx = 0;
                int64_t val = std::stoll(sVal, &idx);
                return val;
            }
            catch (...) {
                return static_cast<int64_t>(AsNumber());
            }
        default:
            return 0;
        }
    }

    std::string Value::AsString() const {
        switch (type) {
        case ValType::Nil:
            return "nil";
        case ValType::Bool:
            return bVal ? "true" : "false";
        case ValType::Int:
            return std::to_string(iVal);
        case ValType::Float: {
            std::string s = std::to_string(fVal);
            if (s.find('.') != std::string::npos) {
                s.erase(s.find_last_not_of('0') + 1);
                if (!s.empty() && s.back() == '.') s.pop_back();
            }
            return s;
        }
        case ValType::String:
            return sVal;
        default:
            return "";
        }
    }

    Value operator+(const Value& lhs, const Value& rhs) {
        if (lhs.type == ValType::String || rhs.type == ValType::String) {
            return Value::Str(lhs.AsString() + rhs.AsString());
        }
        if (lhs.type == ValType::Float || rhs.type == ValType::Float) {
            return Value::Float(lhs.AsNumber() + rhs.AsNumber());
        }
        return Value::Int(lhs.AsInt() + rhs.AsInt());
    }

    Value operator-(const Value& lhs, const Value& rhs) {
        if (lhs.type == ValType::Float || rhs.type == ValType::Float) {
            return Value::Float(lhs.AsNumber() - rhs.AsNumber());
        }
        return Value::Int(lhs.AsInt() - rhs.AsInt());
    }

    Value operator*(const Value& lhs, const Value& rhs) {
        if (lhs.type == ValType::Float || rhs.type == ValType::Float) {
            return Value::Float(lhs.AsNumber() * rhs.AsNumber());
        }
        return Value::Int(lhs.AsInt() * rhs.AsInt());
    }

    Value operator/(const Value& lhs, const Value& rhs) {
        if (lhs.type == ValType::Float || rhs.type == ValType::Float) {
            double r = rhs.AsNumber();
            if (std::abs(r) < 1e-12) return Value::Float(0.0);
            return Value::Float(lhs.AsNumber() / r);
        }
        int64_t r = rhs.AsInt();
        if (r == 0) return Value::Int(0);
        return Value::Int(lhs.AsInt() / r);
    }

    Value operator%(const Value& lhs, const Value& rhs) {
        int64_t r = rhs.AsInt();
        if (r == 0) return Value::Int(0);
        return Value::Int(lhs.AsInt() % r);
    }

    bool ValueEqual(const Value& lhs, const Value& rhs) {
        if (lhs.type == ValType::Nil && rhs.type == ValType::Nil) return true;
        if (lhs.type == ValType::Nil || rhs.type == ValType::Nil) return false;

        if (lhs.type == ValType::String && rhs.type == ValType::String) {
            return lhs.sVal == rhs.sVal;
        }
        if (lhs.type == ValType::Bool && rhs.type == ValType::Bool) {
            return lhs.bVal == rhs.bVal;
        }
        if ((lhs.type == ValType::Int || lhs.type == ValType::Float) &&
            (rhs.type == ValType::Int || rhs.type == ValType::Float)) {
            if (lhs.type == ValType::Int && rhs.type == ValType::Int) {
                return lhs.iVal == rhs.iVal;
            }
            return std::abs(lhs.AsNumber() - rhs.AsNumber()) < 1e-6;
        }
        return false;
    }

    bool ValueLess(const Value& lhs, const Value& rhs) {
        if (lhs.type == ValType::String && rhs.type == ValType::String) {
            return lhs.sVal < rhs.sVal;
        }
        return lhs.AsNumber() < rhs.AsNumber();
    }

} // namespace ar
