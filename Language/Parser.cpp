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

    std::string BlueprintNode::ToString() const {
        std::string result = "Blueprint(" + name;
        if (!parent.empty()) result += " extends " + parent;
        result += ") {\n";
        for (const auto& prop : properties) {
            result += "    " + prop.ToString() + "\n";
        }
        result += "}";
        return result;
    }

    std::string InstanceNode::ToString() const {
        std::string result = "Instance(" + name + " -> " + blueprintName + ") at (" +
            std::to_string(x) + ", " + std::to_string(y) + ") {\n";
        for (const auto& prop : overrides) {
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
        for (const auto& bp : blueprints) {
            result += bp.ToString() + "\n";
        }
        for (const auto& inst : instances) {
            result += inst.ToString() + "\n";
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
            else if (currentToken.type == TokenType::IDENTIFIER &&
                     (currentToken.literal == "on" || currentToken.literal == "after")) {
                if (!ParseEvent()) return false;
            }
            else {
                error = "Parse error at line " + std::to_string(currentToken.line) +
                    ": unexpected token " + TokenTypeToString(currentToken.type) +
                    "('" + currentToken.literal + "'). " +
                    "Expected '#' to start an object, 'on' to start an event, "
                    "or 'after' to start a one-shot event.";
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
        const std::string name = currentToken.literal;
        Advance();

        if (!Expect(TokenType::IDENTIFIER, "object type")) return false;
        const std::string type = currentToken.literal;
        Advance();

        if (type == "Blueprint") {
            return ParseBlueprint(name);
        }

        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "at") {
            return ParseInstance(name, type);
        }

        ObjectNode obj;
        obj.name = name;
        obj.type = type;

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

    bool Parser::ParseBlueprint(const std::string& name) {
        BlueprintNode blueprint;
        blueprint.name = name;

        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "extends") {
            Advance();
            if (!Expect(TokenType::IDENTIFIER, "blueprint parent")) return false;
            blueprint.parent = currentToken.literal;
            Advance();
        }

        SkipNewlines();

        if (!Expect(TokenType::LEFT_BRACE, "blueprint body")) return false;
        Advance();
        SkipNewlines();

        while (currentToken.type != TokenType::RIGHT_BRACE &&
            currentToken.type != TokenType::END_OF_FILE) {
            SkipNewlines();
            if (currentToken.type == TokenType::RIGHT_BRACE) break;

            if (!ParseProperty(blueprint)) return false;
            SkipNewlines();
        }

        if (!Expect(TokenType::RIGHT_BRACE, "blueprint body")) return false;
        Advance();

        ast.blueprints.push_back(blueprint);
        return true;
    }

    bool Parser::ParseInstance(const std::string& name, const std::string& type) {
        if (currentToken.type != TokenType::IDENTIFIER || currentToken.literal != "at") {
            error = "Parse error at line " + std::to_string(currentToken.line) +
                ": expected 'at' after instance type";
            return false;
        }

        InstanceNode instance;
        instance.name = name;
        instance.blueprintName = type;

        Advance(); // consume 'at'

        if (!Expect(TokenType::NUMBER, "instance x position")) return false;
        instance.x = std::stof(currentToken.literal);
        Advance();

        if (!Expect(TokenType::NUMBER, "instance y position")) return false;
        instance.y = std::stof(currentToken.literal);
        Advance();

        SkipNewlines();

        if (currentToken.type == TokenType::LEFT_BRACE) {
            Advance();
            SkipNewlines();
            while (currentToken.type != TokenType::RIGHT_BRACE && currentToken.type != TokenType::END_OF_FILE) {
                SkipNewlines();
                if (currentToken.type == TokenType::RIGHT_BRACE) break;
                if (!ParseProperty(instance)) return false;
                SkipNewlines();
            }
            if (!Expect(TokenType::RIGHT_BRACE, "instance body")) return false;
            Advance();
        }

        ast.instances.push_back(instance);
        return true;
    }

    // property : value
    bool Parser::ParseProperty(ObjectNode& obj) {
        return ParseProperty(obj.properties);
    }

    bool Parser::ParseProperty(BlueprintNode& blueprint) {
        return ParseProperty(blueprint.properties);
    }

    bool Parser::ParseProperty(InstanceNode& instance) {
        return ParseProperty(instance.overrides);
    }

    bool Parser::ParseProperty(std::vector<Property>& properties) {
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

        properties.push_back(prop);
        return true;
    }

    // on Trigger { commands }
    // after N   { commands }   (one-shot, fires once N seconds after level start)
    bool Parser::ParseEvent() {
        // currentToken is either IDENTIFIER "on" or IDENTIFIER "after".
        std::string keyword = currentToken.literal;
        Advance(); // consume keyword

        EventNode event;
        std::string triggerStr;

        while (currentToken.type == TokenType::IDENTIFIER || currentToken.type == TokenType::NUMBER) {
            if (!triggerStr.empty()) triggerStr += " ";
            triggerStr += currentToken.literal;
            Advance();
        }

        if (triggerStr.empty()) {
            error = "Parse error at line " + std::to_string(currentToken.line) +
                ": expected event trigger after '" + keyword + "'";
            return false;
        }

        // Preserve the "after" prefix so RuntimeBridge can distinguish
        // one-shot "after N" events from repeating "timer N" events.
        if (keyword == "after") {
            event.trigger = "after " + triggerStr;
        } else {
            event.trigger = triggerStr;
        }

        SkipNewlines();

        if (!Expect(TokenType::LEFT_BRACE, "event body")) return false;
        Advance();
        SkipNewlines();

        while (currentToken.type != TokenType::RIGHT_BRACE &&
            currentToken.type != TokenType::END_OF_FILE) {
            SkipNewlines();
            if (currentToken.type == TokenType::RIGHT_BRACE) break;

            if (currentToken.type != TokenType::IDENTIFIER && currentToken.type != TokenType::NUMBER) {
                error = "Parse error at line " + std::to_string(currentToken.line) +
                    ": expected event command, got " + TokenTypeToString(currentToken.type) +
                    "('" + currentToken.literal + "')";
                return false;
            }
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