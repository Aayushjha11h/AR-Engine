#include "Interpreter.h"
#include <unordered_set>
#include <sstream>

namespace ar {

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

    Interpreter::Interpreter() = default;

    bool Interpreter::Load(const AST& ast) {
        objects.clear();
        events.clear();
        blueprints.clear();
        blueprintDefs.clear();
        error.clear();

        for (const auto& node : ast.blueprints) {
            if (blueprintDefs.count(node.name)) {
                error = "Interpreter error: duplicate blueprint name '" + node.name + "'";
                return false;
            }
            blueprintDefs[node.name] = node;
        }

        for (const auto& node : ast.blueprints) {
            std::unordered_set<std::string> visiting;
            ScriptObject resolved = ResolveBlueprint(node.name, visiting);
            if (resolved.name.empty() && resolved.type.empty()) {
                return false;
            }
        }

        for (const auto& objNode : ast.objects) {
            if (!ProcessObject(objNode)) return false;
        }

        for (const auto& instNode : ast.instances) {
            if (!ProcessInstance(instNode)) return false;
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

    const ScriptObject* Interpreter::GetBlueprint(const std::string& name) const {
        auto it = blueprints.find(name);
        if (it == blueprints.end()) return nullptr;
        return &it->second;
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
            obj.properties[prop.name] = prop.value.value;
        }

        objects.push_back(obj);
        return true;
    }

    ScriptObject Interpreter::ResolveBlueprint(const std::string& name,
        std::unordered_set<std::string>& visiting) {
        auto cached = blueprints.find(name);
        if (cached != blueprints.end()) {
            return cached->second;
        }

        auto itDef = blueprintDefs.find(name);
        if (itDef == blueprintDefs.end()) {
            error = "Interpreter error: blueprint '" + name + "' not found";
            return ScriptObject{};
        }

        if (visiting.find(name) != visiting.end()) {
            std::string chain;
            for (const auto& v : visiting) chain += v + " -> ";
            error = "Interpreter error: circular blueprint inheritance: " + chain + name;
            return ScriptObject{};
        }

        visiting.insert(name);
        ScriptObject resolved;
        resolved.name = name;
        resolved.type = "Blueprint";

        if (!itDef->second.parent.empty()) {
            ScriptObject parent = ResolveBlueprint(itDef->second.parent, visiting);
            if (parent.name.empty() && parent.type.empty()) {
                visiting.erase(name);
                return ScriptObject{};
            }
            resolved.properties = parent.properties;
        }

        for (const auto& prop : itDef->second.properties) {
            resolved.properties[prop.name] = prop.value.value;
        }

        visiting.erase(name);
        blueprints[name] = resolved;
        return resolved;
    }

    bool Interpreter::ProcessInstance(const InstanceNode& node) {
        std::unordered_set<std::string> visiting;
        ScriptObject resolved = ResolveBlueprint(node.blueprintName, visiting);
        if (resolved.name.empty() && resolved.type.empty()) {
            return false;
        }

        ScriptObject instance = resolved;
        instance.name = node.name;
        instance.type = node.blueprintName;

        auto fmt = [](float v) {
            std::ostringstream ss;
            ss << v;
            return ss.str();
            };
        instance.properties["x"] = fmt(node.x);
        instance.properties["y"] = fmt(node.y);

        for (const auto& prop : node.overrides) {
            instance.properties[prop.name] = prop.value.value;
        }

        for (const auto& existing : objects) {
            if (existing.name == instance.name) {
                error = "Interpreter error: duplicate object name '" + instance.name + "'";
                return false;
            }
        }

        objects.push_back(instance);
        return true;
    }

    bool Interpreter::ProcessEvent(const EventNode& node) {
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