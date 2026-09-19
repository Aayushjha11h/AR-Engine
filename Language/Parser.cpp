#include "Parser.h"

namespace ar {

    // -------------------------------------------------------------------
    // AST Debug Strings
    // -------------------------------------------------------------------

    static std::string ValueTypeToString(ValueType type) {
        switch (type) {
        case ValueType::Number:     return "Number";
        case ValueType::String:     return "String";
        case ValueType::Identifier: return "Identifier";
        default:                    return "Unknown";
        }
    }

    std::string PropertyValue::ToString() const {
        return ValueTypeToString(type) + "(" + value + ")";
    }

    std::string Property::ToString() const {
        return name + " : " + value.ToString();
    }

    std::string ObjectNode::ToString() const {
        std::string result = "Object(" + name + ", " + type + ") {\n";
        for (const auto& prop : properties) {
            result += "    " + prop.ToString() + "\n";
        }
        result += "}";
        return result;
    }

    std::string EventNode::ToString() const {
        std::string result = "Event(" + trigger + ") {\n";
        for (const auto& cmd : commands) {
            result += "    " + cmd + "\n";
        }
        result += "}";
        return result;
    }

    std::string AST::ToString() const {
        std::string result = "AST:\n";
        for (const auto& obj : objects) {
            result += obj.ToString() + "\n";
        }
        for (const auto& evt : events) {
            result += evt.ToString() + "\n";
        }
        return result;
    }

    // -------------------------------------------------------------------
    // Parser Implementation
    // -------------------------------------------------------------------

    Parser::Parser(Lexer& lexer) : lexer(lexer) {
        currentToken = Token(); // safe default until first Advance()
    }

    void Parser::Advance() {
        currentToken = lexer.NextToken();
    }

    bool Parser::Expect(TokenType type, const std::string& context) {
        if (currentToken.type != type) {
            error = "Parse error at line " + std::to_string(currentToken.line) +
                " in " + context + ": expected " + TokenTypeToString(type) +
                ", got " + TokenTypeToString(currentToken.type) +
                "('" + currentToken.literal + "')";
            return false;
        }
        return true;
    }

    void Parser::SkipNewlines() {
        while (currentToken.type == TokenType::NEWLINE) {
            Advance();
        }
    }

    bool Parser::Parse() {
        Advance();
        SkipNewlines();

        while (currentToken.type != TokenType::END_OF_FILE) {
            if (currentToken.type == TokenType::HASH) {
                if (!ParseObject()) return false;
            }
            else if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "on") {
                if (!ParseEvent()) return false;
            }
            else {
                error = "Parse error at line " + std::to_string(currentToken.line) +
                    ": unexpected token " + TokenTypeToString(currentToken.type) +
                    "('" + currentToken.literal + "'). " +
                    "Expected '#' to start an object or 'on' to start an event.";
                return false;
            }
            SkipNewlines();
        }

        return true;
    }

    // #Name Type { properties }
    bool Parser::ParseObject() {
        // currentToken is HASH
        Advance(); // consume '#'

        if (!Expect(TokenType::IDENTIFIER, "object name")) return false;
        ObjectNode obj;
        obj.name = currentToken.literal;
        Advance();

        if (!Expect(TokenType::IDENTIFIER, "object type")) return false;
        obj.type = currentToken.literal;
        Advance();

        SkipNewlines();

        if (!Expect(TokenType::LEFT_BRACE, "object body")) return false;
        Advance();
        SkipNewlines();

        while (currentToken.type != TokenType::RIGHT_BRACE &&
            currentToken.type != TokenType::END_OF_FILE) {
            SkipNewlines();
            if (currentToken.type == TokenType::RIGHT_BRACE) break;

            if (!ParseProperty(obj)) return false;
            SkipNewlines();
        }

        if (!Expect(TokenType::RIGHT_BRACE, "object body")) return false;
        Advance();

        ast.objects.push_back(obj);
        return true;
    }

    // property : value
    bool Parser::ParseProperty(ObjectNode& obj) {
        if (!Expect(TokenType::IDENTIFIER, "property name")) return false;
        Property prop;
        prop.name = currentToken.literal;
        Advance();

        if (!Expect(TokenType::COLON, "property separator")) return false;
        Advance();

        ValueType vtype;
        if (currentToken.type == TokenType::NUMBER) {
            vtype = ValueType::Number;
        }
        else if (currentToken.type == TokenType::STRING) {
            vtype = ValueType::String;
        }
        else if (currentToken.type == TokenType::IDENTIFIER) {
            vtype = ValueType::Identifier;
        }
        else {
            error = "Parse error at line " + std::to_string(currentToken.line) +
                ": expected property value (number, string, or identifier), got " +
                TokenTypeToString(currentToken.type) +
                "('" + currentToken.literal + "')";
            return false;
        }

        prop.value = { vtype, currentToken.literal };
        Advance();

        obj.properties.push_back(prop);
        return true;
    }

    // on Trigger { commands }
    bool Parser::ParseEvent() {
        // currentToken is IDENTIFIER "on"
        Advance(); // consume "on"

        if (!Expect(TokenType::IDENTIFIER, "event trigger")) return false;
        EventNode event;
        event.trigger = currentToken.literal;
        Advance();

        SkipNewlines();

        if (!Expect(TokenType::LEFT_BRACE, "event body")) return false;
        Advance();
        SkipNewlines();

        while (currentToken.type != TokenType::RIGHT_BRACE &&
            currentToken.type != TokenType::END_OF_FILE) {
            SkipNewlines();
            if (currentToken.type == TokenType::RIGHT_BRACE) break;

            if (!Expect(TokenType::IDENTIFIER, "event command")) return false;
            event.commands.push_back(currentToken.literal);
            Advance();
            SkipNewlines();
        }

        if (!Expect(TokenType::RIGHT_BRACE, "event body")) return false;
        Advance();

        ast.events.push_back(event);
        return true;
    }

} // namespace ar