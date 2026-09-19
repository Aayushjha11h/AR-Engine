#pragma once
#include "Token.h"
#include "Lexer.h"
#include <vector>
#include <string>

namespace ar {

    // -------------------------------------------------------------------
    // AST Node Definitions
    // -------------------------------------------------------------------

    // The three kinds of values a property can hold in AR GDL MVP.
    enum class ValueType {
        Number,      // e.g., 200, 3.14
        String,      // e.g., "hero.png"
        Identifier   // e.g., on, off, true
    };

    // Right-hand side of a property assignment: speed : 200
    struct PropertyValue {
        ValueType type;
        std::string value;  // Raw literal: "200", "hero.png", "on"

        std::string ToString() const;
    };

    // A single property: name : value
    struct Property {
        std::string name;
        PropertyValue value;

        std::string ToString() const;
    };

    // #Name Type { properties }
    struct ObjectNode {
        std::string name;
        std::string type;
        std::vector<Property> properties;

        std::string ToString() const;
    };

    // on Trigger { commands }
    struct EventNode {
        std::string trigger;
        std::vector<std::string> commands;

        std::string ToString() const;
    };

    // Root AST container returned after parsing.
    struct AST {
        std::vector<ObjectNode> objects;
        std::vector<EventNode> events;

        std::string ToString() const;
    };

    // -------------------------------------------------------------------
    // Parser
    // -------------------------------------------------------------------

    class Parser {
    public:
        explicit Parser(Lexer& lexer);

        // Parse the entire token stream. Returns true on success.
        bool Parse();

        // Access the parsed AST.
        const AST& GetAST() const { return ast; }

        // If Parse() returned false, this holds the error message.
        const std::string& GetError() const { return error; }

    private:
        Lexer& lexer;
        Token currentToken;
        AST ast;
        std::string error;

        // Pull the next token from the lexer.
        void Advance();

        // Verify current token is 'type'. If not, set error and return false.
        bool Expect(TokenType type, const std::string& context);

        // Consume any NEWLINE tokens. AR GDL is newline-separated but not newline-sensitive.
        void SkipNewlines();

        // Grammar handlers
        bool ParseObject();
        bool ParseEvent();
        bool ParseProperty(ObjectNode& object);
    };

} // namespace ar