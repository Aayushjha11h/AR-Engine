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
        result += "    // statements (" + std::to_string(body.size()) + ")\n";
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

    // on Trigger [if Guard] { body }
    // after N   { body }   (one-shot, fires once N seconds after level start)
    bool Parser::ParseEvent() {
        std::string keyword = currentToken.literal;
        Advance(); // consume keyword ("on" or "after")

        EventNode event;
        std::string triggerStr;

        while (currentToken.type == TokenType::IDENTIFIER || currentToken.type == TokenType::NUMBER) {
            if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "if" && !triggerStr.empty()) {
                break;
            }
            if (!triggerStr.empty()) triggerStr += " ";
            triggerStr += currentToken.literal;
            Advance();
        }

        if (triggerStr.empty()) {
            error = "Parse error at line " + std::to_string(currentToken.line) +
                ": expected event trigger after '" + keyword + "'";
            return false;
        }

        if (keyword == "after") {
            event.trigger = "after " + triggerStr;
        } else {
            event.trigger = triggerStr;
        }

        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "if") {
            Advance();
            event.guard = ParseExpr();
            if (!event.guard) {
                if (error.empty()) {
                    error = "Parse error at line " + std::to_string(currentToken.line) +
                        ": invalid expression in event guard";
                }
                return false;
            }
        }

        SkipNewlines();

        if (!Expect(TokenType::LEFT_BRACE, "event body")) return false;
        Advance();
        SkipNewlines();

        if (!ParseBlock(event.body)) return false;

        if (!Expect(TokenType::RIGHT_BRACE, "event body")) return false;
        Advance();

        ast.events.push_back(std::move(event));
        return true;
    }

    bool Parser::ParseBlock(std::vector<Statement>& body) {
        while (currentToken.type != TokenType::RIGHT_BRACE &&
            currentToken.type != TokenType::END_OF_FILE) {
            SkipNewlines();
            if (currentToken.type == TokenType::RIGHT_BRACE) break;

            if (!ParseStatement(body)) {
                return false;
            }
            SkipNewlines();
        }
        return true;
    }

    bool Parser::ParseStatement(std::vector<Statement>& body) {
        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "set") {
            Advance(); // consume "set"
            if (currentToken.type == TokenType::DOLLAR) {
                Advance();
            }
            if (!Expect(TokenType::IDENTIFIER, "variable name in set")) return false;
            std::string varName = currentToken.literal;
            Advance();

            if (!Expect(TokenType::EQUAL, "'=' in set statement")) return false;
            Advance();

            auto valExpr = ParseExpr();
            if (!valExpr) return false;

            body.push_back(Statement::MakeSet(varName, std::move(valExpr)));
            return true;
        }

        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "if") {
            Advance(); // consume "if"
            auto cond = ParseExpr();
            if (!cond) return false;

            if (!Expect(TokenType::LEFT_BRACE, "'{' after if condition")) return false;
            Advance();

            std::vector<Statement> ifBody;
            if (!ParseBlock(ifBody)) return false;

            if (!Expect(TokenType::RIGHT_BRACE, "'}' closing if body")) return false;
            Advance();

            std::vector<Statement> elseBody;
            SkipNewlines();
            if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "else") {
                Advance(); // consume "else"
                SkipNewlines();
                if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "if") {
                    if (!ParseStatement(elseBody)) return false;
                }
                else {
                    if (!Expect(TokenType::LEFT_BRACE, "'{' after else")) return false;
                    Advance();
                    if (!ParseBlock(elseBody)) return false;
                    if (!Expect(TokenType::RIGHT_BRACE, "'}' closing else body")) return false;
                    Advance();
                }
            }

            body.push_back(Statement::MakeIf(std::move(cond), std::move(ifBody), std::move(elseBody)));
            return true;
        }

        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "while") {
            Advance(); // consume "while"
            auto cond = ParseExpr();
            if (!cond) return false;

            if (!Expect(TokenType::LEFT_BRACE, "'{' after while condition")) return false;
            Advance();

            std::vector<Statement> whileBody;
            if (!ParseBlock(whileBody)) return false;

            if (!Expect(TokenType::RIGHT_BRACE, "'}' closing while body")) return false;
            Advance();

            body.push_back(Statement::MakeWhile(std::move(cond), std::move(whileBody)));
            return true;
        }

        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "for") {
            Advance(); // consume "for"
            if (!Expect(TokenType::IDENTIFIER, "loop variable name in for")) return false;
            std::string varName = currentToken.literal;
            Advance();

            if (!Expect(TokenType::IDENTIFIER, "'in' after for loop variable") || currentToken.literal != "in") {
                error = "Parse error: expected 'in' in for loop";
                return false;
            }
            Advance();

            if (!Expect(TokenType::IDENTIFIER, "'range' after 'in'") || currentToken.literal != "range") {
                error = "Parse error: expected 'range' in for loop";
                return false;
            }
            Advance();

           auto startExpr = ParseExpr();
            if (!startExpr) return false;

           auto endExpr = ParseExpr();
            if (!endExpr) return false;

            if (!Expect(TokenType::LEFT_BRACE, "'{' after for range")) return false;
            Advance();

            std::vector<Statement> forBody;
            if (!ParseBlock(forBody)) return false;

            if (!Expect(TokenType::RIGHT_BRACE, "'}' closing for body")) return false;
            Advance();

            body.push_back(Statement::MakeFor(varName, std::move(startExpr), std::move(endExpr), std::move(forBody)));
            return true;
        }

        // Collect tokens until newline or RIGHT_BRACE as a Command
        std::vector<std::string> tokens;
        while (currentToken.type != TokenType::NEWLINE &&
            currentToken.type != TokenType::RIGHT_BRACE &&
            currentToken.type != TokenType::END_OF_FILE) {
            if (currentToken.type == TokenType::DOLLAR) {
                Advance();
                if (currentToken.type == TokenType::IDENTIFIER) {
                    tokens.push_back("$" + currentToken.literal);
                    Advance();
                }
                else {
                    tokens.push_back("$");
                }
            }
            else {
                tokens.push_back(currentToken.literal);
                Advance();
            }
        }
        if (!tokens.empty()) {
            body.push_back(Statement::MakeCommand(std::move(tokens)));
        }
        return true;
    }

    std::unique_ptr<Expr> Parser::ParseExpr() {
        return ParseOr();
    }

    std::unique_ptr<Expr> Parser::ParseOr() {
        auto expr = ParseAnd();
        if (!expr) return nullptr;

        while (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "or") {
            Advance();
            auto rhs = ParseAnd();
            if (!rhs) return nullptr;
            expr = Expr::MakeBinary("or", std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::ParseAnd() {
        auto expr = ParseEquality();
        if (!expr) return nullptr;

        while (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "and") {
            Advance();
            auto rhs = ParseEquality();
            if (!rhs) return nullptr;
            expr = Expr::MakeBinary("and", std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::ParseEquality() {
        auto expr = ParseComparison();
        if (!expr) return nullptr;

        while (currentToken.type == TokenType::EQUAL_EQUAL || currentToken.type == TokenType::NOT_EQUAL) {
            std::string op = currentToken.literal;
            Advance();
            auto rhs = ParseComparison();
            if (!rhs) return nullptr;
            expr = Expr::MakeBinary(op, std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::ParseComparison() {
        auto expr = ParseAdditive();
        if (!expr) return nullptr;

        while (currentToken.type == TokenType::LESS ||
               currentToken.type == TokenType::LESS_EQUAL ||
               currentToken.type == TokenType::GREATER ||
               currentToken.type == TokenType::GREATER_EQUAL) {
            std::string op = currentToken.literal;
            Advance();
            auto rhs = ParseAdditive();
            if (!rhs) return nullptr;
            expr = Expr::MakeBinary(op, std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::ParseAdditive() {
        auto expr = ParseMultiplicative();
        if (!expr) return nullptr;

        while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
            std::string op = currentToken.literal;
            Advance();
            auto rhs = ParseMultiplicative();
            if (!rhs) return nullptr;
            expr = Expr::MakeBinary(op, std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::ParseMultiplicative() {
        auto expr = ParseUnary();
        if (!expr) return nullptr;

        while (currentToken.type == TokenType::STAR ||
               currentToken.type == TokenType::SLASH ||
               currentToken.type == TokenType::PERCENT) {
            std::string op = currentToken.literal;
            Advance();
            auto rhs = ParseUnary();
            if (!rhs) return nullptr;
            expr = Expr::MakeBinary(op, std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::ParseUnary() {
        if (currentToken.type == TokenType::MINUS) {
            Advance();
            auto operand = ParseUnary();
            if (!operand) return nullptr;
            return Expr::MakeUnary("-", std::move(operand));
        }
        if (currentToken.type == TokenType::IDENTIFIER && currentToken.literal == "not") {
            Advance();
            auto operand = ParseUnary();
            if (!operand) return nullptr;
            return Expr::MakeUnary("not", std::move(operand));
        }
        return ParsePrimary();
    }

    std::unique_ptr<Expr> Parser::ParsePrimary() {
        if (currentToken.type == TokenType::NUMBER) {
            bool hasDot = (currentToken.literal.find('.') != std::string::npos);
            Value val = hasDot ? Value::Float(std::stod(currentToken.literal))
                               : Value::Int(std::stoll(currentToken.literal));
            Advance();
            return Expr::MakeLiteral(val);
        }

        if (currentToken.type == TokenType::STRING) {
            Value val = Value::Str(currentToken.literal);
            Advance();
            return Expr::MakeLiteral(val);
        }

        if (currentToken.type == TokenType::DOLLAR) {
            Advance();
            if (currentToken.type != TokenType::IDENTIFIER) {
                error = "Parse error at line " + std::to_string(currentToken.line) +
                    ": expected variable name after '$'";
                return nullptr;
            }
            std::string name = currentToken.literal;
            Advance();
            return Expr::MakeVar(name);
        }

        if (currentToken.type == TokenType::LEFT_PAREN) {
            Advance();
            auto inner = ParseExpr();
            if (!inner) return nullptr;
            if (!Expect(TokenType::RIGHT_PAREN, "closing ')'")) return nullptr;
            Advance();
            return inner;
        }

        if (currentToken.type == TokenType::IDENTIFIER) {
            if (currentToken.literal == "true") {
                Advance();
                return Expr::MakeLiteral(Value::Bool(true));
            }
            if (currentToken.literal == "false") {
                Advance();
                return Expr::MakeLiteral(Value::Bool(false));
            }
            if (currentToken.literal == "nil") {
                Advance();
                return Expr::MakeLiteral(Value::Nil());
            }

            if (currentToken.literal == "exists") {
                Advance();
                std::vector<std::unique_ptr<Expr>> args;
                if (currentToken.type == TokenType::LEFT_PAREN) {
                    Advance();
                    auto arg = ParseExpr();
                    if (!arg) return nullptr;
                    if (!Expect(TokenType::RIGHT_PAREN, "')' in exists")) return nullptr;
                    Advance();
                    args.push_back(std::move(arg));
                }
                else {
                    auto arg = ParsePrimary();
                    if (!arg) return nullptr;
                    args.push_back(std::move(arg));
                }
                return Expr::MakeCall("exists", std::move(args));
            }

            if (currentToken.literal == "distance_to") {
                Advance();
                std::vector<std::unique_ptr<Expr>> args;
                if (currentToken.type == TokenType::LEFT_PAREN) {
                    Advance();
                    auto arg = ParseExpr();
                    if (!arg) return nullptr;
                    if (!Expect(TokenType::RIGHT_PAREN, "')' in distance_to")) return nullptr;
                    Advance();
                    args.push_back(std::move(arg));
                }
                else {
                    auto arg = ParsePrimary();
                    if (!arg) return nullptr;
                    args.push_back(std::move(arg));
                }
                return Expr::MakeCall("distance_to", std::move(args));
            }

            if (currentToken.literal == "random") {
                Advance();
                std::vector<std::unique_ptr<Expr>> args;
                if (currentToken.type == TokenType::LEFT_PAREN) {
                    Advance();
                    auto lo = ParseExpr();
                    if (!lo) return nullptr;
                    if (currentToken.type == TokenType::COMMA) Advance();
                    auto hi = ParseExpr();
                    if (!hi) return nullptr;
                    if (!Expect(TokenType::RIGHT_PAREN, "')' in random")) return nullptr;
                    Advance();
                    args.push_back(std::move(lo));
                    args.push_back(std::move(hi));
                }
                else {
                    auto lo = ParsePrimary();
                    if (!lo) return nullptr;
                    auto hi = ParsePrimary();
                    if (!hi) return nullptr;
                    args.push_back(std::move(lo));
                    args.push_back(std::move(hi));
                }
                return Expr::MakeCall("random", std::move(args));
            }

            std::string id = currentToken.literal;
            Advance();

            if (currentToken.type == TokenType::LEFT_PAREN) {
                Advance();
                std::vector<std::unique_ptr<Expr>> args;
                if (currentToken.type != TokenType::RIGHT_PAREN) {
                    while (true) {
                        auto arg = ParseExpr();
                        if (!arg) return nullptr;
                        args.push_back(std::move(arg));
                        if (currentToken.type == TokenType::COMMA) {
                            Advance();
                        }
                        else {
                            break;
                        }
                    }
                }
                if (!Expect(TokenType::RIGHT_PAREN, "')' after function call arguments")) return nullptr;
                Advance();
                return Expr::MakeCall(id, std::move(args));
            }

            size_t dotPos = id.find('.');
            if (dotPos != std::string::npos) {
                return Expr::MakeMember(id.substr(0, dotPos), id.substr(dotPos + 1));
            }

            return Expr::MakeLiteral(Value::Str(id));
        }

        error = "Parse error at line " + std::to_string(currentToken.line) +
            ": unexpected token in expression: " + TokenTypeToString(currentToken.type) +
            "('" + currentToken.literal + "')";
        return nullptr;
    }

} // namespace ar