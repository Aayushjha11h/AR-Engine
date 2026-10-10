#pragma once
#include "Parser.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace ar {

    // -------------------------------------------------------------------
    // Runtime Structures
    // -------------------------------------------------------------------

    // Flattened object definition ready for the Runtime Bridge.
    // All property values are stored as strings; the bridge converts
    // them to engine types (int, float, bool, path) as needed.
    struct ScriptObject {
        std::string name;
        std::string type;
        std::unordered_map<std::string, std::string> properties;

        std::string ToString() const;
    };

    // Flattened event handler.
    struct ScriptEvent {
        std::string trigger;
        std::unique_ptr<Expr> guard;
        std::vector<Statement> body;

        ScriptEvent() = default;
        ~ScriptEvent() = default;
        ScriptEvent(const ScriptEvent&) = delete;
        ScriptEvent& operator=(const ScriptEvent&) = delete;
        ScriptEvent(ScriptEvent&&) noexcept = default;
        ScriptEvent& operator=(ScriptEvent&&) noexcept = default;

        std::string ToString() const;
    };

    // -------------------------------------------------------------------
    // Interpreter
    // -------------------------------------------------------------------

    class Interpreter {
    public:
        Interpreter();

        // Load and validate the AST.
        // Takes non-const AST reference to move nodes without deep copies.
        bool Load(AST& ast);

        // --- API for the Runtime Bridge ---

        const std::vector<ScriptObject>& GetObjects() const { return objects; }
        const std::vector<ScriptEvent>& GetEvents() const { return events; }
        const std::unordered_map<std::string, ScriptObject>& GetBlueprints() const { return blueprints; }

        // Look up a specific object by name. Returns nullptr if not found.
        const ScriptObject* GetObject(const std::string& name) const;
        const ScriptObject* GetBlueprint(const std::string& name) const;

        // Error reporting
        bool HasError() const { return !error.empty(); }
        const std::string& GetError() const { return error; }

    private:
        std::vector<ScriptObject> objects;
        std::vector<ScriptEvent> events;
        std::unordered_map<std::string, ScriptObject> blueprints;
        std::unordered_map<std::string, BlueprintNode> blueprintDefs;
        std::string error;

        bool ProcessObject(const ObjectNode& node);
       
        bool ProcessInstance(const InstanceNode& node);
        bool ProcessEvent(EventNode& node);
        ScriptObject ResolveBlueprint(const std::string& name,
            std::unordered_set<std::string>& visiting);
    };

} // namespace ar