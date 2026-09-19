#pragma once
#include "Parser.h"
#include <vector>
#include <string>
#include <unordered_map>
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
    // Commands are stored exactly as the Parser produced them.
    struct ScriptEvent {
        std::string trigger;
        std::vector<std::string> commands;

        std::string ToString() const;
    };

    // -------------------------------------------------------------------
    // Interpreter
    // -------------------------------------------------------------------

    class Interpreter {
    public:
        Interpreter();

        // Load and validate the AST.
        // Returns true if the script is well-formed.
        bool Load(const AST& ast);

        // --- API for the Runtime Bridge ---

        const std::vector<ScriptObject>& GetObjects() const { return objects; }
        const std::vector<ScriptEvent>& GetEvents() const { return events; }

        // Look up a specific object by name. Returns nullptr if not found.
        const ScriptObject* GetObject(const std::string& name) const;

        // Look up event commands by trigger. Returns nullptr if not found.
        const std::vector<std::string>* GetEventCommands(const std::string& trigger) const;

        // Execute every command in an event by calling 'handler' once per command.
        // The handler is provided by the Runtime Bridge.
        void ExecuteEvent(const std::string& trigger,
            const std::function<void(const std::string&)>& handler) const;

        // Error reporting
        bool HasError() const { return !error.empty(); }
        const std::string& GetError() const { return error; }

    private:
        std::vector<ScriptObject> objects;
        std::vector<ScriptEvent> events;
        std::string error;

        bool ProcessObject(const ObjectNode& node);
        bool ProcessEvent(const EventNode& node);
    };

} // namespace ar