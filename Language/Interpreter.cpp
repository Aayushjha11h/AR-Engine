#include "Interpreter.h"

namespace ar {

    // -------------------------------------------------------------------
    // Debug Strings
    // -------------------------------------------------------------------

    std::string ScriptObject::ToString() const {
        std::string result = "ScriptObject(" + name + ", " + type + ") {\n";
        for (const auto& pair : properties) {
            result += "    " + pair.first + " : " + pair.second + "\n";
        }
        result += "}";
        return result;
    }

    std::string ScriptEvent::ToString() const {
        std::string result = "ScriptEvent(" + trigger + ") {\n";
        for (const auto& cmd : commands) {
            result += "    " + cmd + "\n";
        }
        result += "}";
        return result;
    }

    // -------------------------------------------------------------------
    // Interpreter Implementation
    // -------------------------------------------------------------------

    Interpreter::Interpreter() = default;

    bool Interpreter::Load(const AST& ast) {
        objects.clear();
        events.clear();
        error.clear();

        for (const auto& objNode : ast.objects) {
            if (!ProcessObject(objNode)) return false;
        }

        for (const auto& evtNode : ast.events) {
            if (!ProcessEvent(evtNode)) return false;
        }

        return true;
    }

    const ScriptObject* Interpreter::GetObject(const std::string& name) const {
        for (const auto& obj : objects) {
            if (obj.name == name) return &obj;
        }
        return nullptr;
    }

    const std::vector<std::string>* Interpreter::GetEventCommands(const std::string& trigger) const {
        for (const auto& evt : events) {
            if (evt.trigger == trigger) return &evt.commands;
        }
        return nullptr;
    }

    void Interpreter::ExecuteEvent(const std::string& trigger,
        const std::function<void(const std::string&)>& handler) const {
        const auto* commands = GetEventCommands(trigger);
        if (!commands) return;
        for (const auto& cmd : *commands) {
            handler(cmd);
        }
    }

    bool Interpreter::ProcessObject(const ObjectNode& node) {
        // MVP validation: no duplicate object names
        for (const auto& existing : objects) {
            if (existing.name == node.name) {
                error = "Interpreter error: duplicate object name '" + node.name + "'";
                return false;
            }
        }

        ScriptObject obj;
        obj.name = node.name;
        obj.type = node.type;

        for (const auto& prop : node.properties) {
            // Flatten PropertyValue to string.
            // Numbers, identifiers, and strings all become raw strings for the bridge.
            obj.properties[prop.name] = prop.value.value;
        }

        objects.push_back(obj);
        return true;
    }

    bool Interpreter::ProcessEvent(const EventNode& node) {
        // If the same trigger already exists, merge the commands.
        for (auto& existing : events) {
            if (existing.trigger == node.trigger) {
                existing.commands.insert(existing.commands.end(),
                    node.commands.begin(),
                    node.commands.end());
                return true;
            }
        }

        ScriptEvent evt;
        evt.trigger = node.trigger;
        evt.commands = node.commands;
        events.push_back(evt);
        return true;
    }

} // namespace ar