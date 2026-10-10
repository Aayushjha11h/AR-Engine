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
        result += "    // statements\n";
        result += "}";
        return result;
    }

    Interpreter::Interpreter() = default;

    bool Interpreter::Load(AST& ast) {
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

        for (auto& evtNode : ast.events) {
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

    bool Interpreter::ProcessEvent(EventNode& node) {
        if (!node.guard) {
            for (auto& existing : events) {
                if (existing.trigger == node.trigger && !existing.guard) {
                    existing.body.insert(existing.body.end(),
                        std::make_move_iterator(node.body.begin()),
                        std::make_move_iterator(node.body.end()));
                    return true;
                }
            }
        }

        ScriptEvent evt;
        evt.trigger = node.trigger;
        evt.guard = std::move(node.guard);
        evt.body = std::move(node.body);
        events.push_back(std::move(evt));
        return true;
    }

} // namespace ar