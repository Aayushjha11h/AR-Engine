#pragma once
#include "Token.h"
#include "Lexer.h"
#include "Statement.h"
#include "Expr.h"
#include <vector>
#include <string>
#include <memory>

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

    // #Name Blueprint extends Parent { properties }
    struct BlueprintNode {
        std::string name;
        std::string parent;
        std::vector<Property> properties;

        std::string ToString() const;
    };

    // #Name BlueprintName at x y { overrides }
    struct InstanceNode {
        std::string name;
        std::string blueprintName;
        float x = 0.0f;
        float y = 0.0f;
        std::vector<Property> overrides;

        std::string ToString() const;
    };

    // on Trigger [if Guard] { body }
    struct EventNode {
        std::string trigger;
        std::unique_ptr<Expr> guard;
        std::vector<Statement> body;

        EventNode() = default;
        ~EventNode() = default;
        EventNode(const EventNode&) = delete;
        EventNode& operator=(const EventNode&) = delete;
        EventNode(EventNode&&) noexcept = default;
        EventNode& operator=(EventNode&&) noexcept = default;

        std::string ToString() const;
    };

    // Root AST container returned after parsing.
    struct AST {
        std::vector<ObjectNode> objects;
        std::vector<BlueprintNode> blueprints;
        std::vector<InstanceNode> instances;
        std::vector<EventNode> events;

        AST() = default;
        ~AST() = default;
        AST(const AST&) = delete;
        AST& operator=(const AST&) = delete;
        AST(AST&&) noexcept = default;
        AST& operator=(AST&&) noexcept = default;

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
        AST& GetAST() { return ast; }
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
        bool ParseBlueprint(const std::string& name);
        bool ParseInstance(const std::string& name, const std::string& type);
        bool ParseEvent();
        bool ParseProperty(ObjectNode& object);
        bool ParseProperty(BlueprintNode& blueprint);
        bool ParseProperty(InstanceNode& instance);
        bool ParseProperty(std::vector<Property>& properties);

        bool ParseBlock(std::vector<Statement>& body);
        bool ParseStatement(std::vector<Statement>& body);

        std::unique_ptr<Expr> ParseExpr();
        std::unique_ptr<Expr> ParseOr();
        std::unique_ptr<Expr> ParseAnd();
        std::unique_ptr<Expr> ParseEquality();
        std::unique_ptr<Expr> ParseComparison();
        std::unique_ptr<Expr> ParseAdditive();
        std::unique_ptr<Expr> ParseMultiplicative();
        std::unique_ptr<Expr> ParseUnary();
        std::unique_ptr<Expr> ParsePrimary();
    };

} // namespace ar