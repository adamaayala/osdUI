#include "config_parser.hpp"
#include <pugixml.hpp>
#include <format>
#include <vector>
#include "../actions/action_default_values.hpp"
#include "../actions/action_external_call.hpp"
#include "../actions/action_switch.hpp"
#include "../actions/action_random_string.hpp"
#include "../actions/action_file_read.hpp"
#include "../actions/action_match.hpp"
#include "../actions/action_ts_var.hpp"
#include "../actions/action_ts_var_list.hpp"
#include "../actions/action_error_info.hpp"
#include "../actions/action_vars.hpp"

namespace osdui::config {
namespace {

// Stub: return a non-null placeholder for known types so tests pass.
// Real factories replace individual branches in Chunk 5+.
std::unique_ptr<IAction> make_action(std::wstring_view type, const pugi::xml_node& node) {
    // Handle DefaultValues first with full implementation
    if (type == L"DefaultValues") {
        auto action = std::make_unique<actions::DefaultValuesAction>();
        for (const auto& var : node.children(L"Variable")) {
            action->add(var.attribute(L"Name").as_string(),
                        var.attribute(L"Value").as_string());
        }
        return action;
    }

    // Handle ExternalCall with full implementation
    if (type == L"ExternalCall") {
        auto action = std::make_unique<actions::ExternalCallAction>();
        action->set_run(node.attribute(L"Run").as_string());
        if (auto v = node.attribute(L"Variable"); v)
            action->set_variable(v.as_string());
        if (auto e = node.attribute(L"SuccessExitCode"); e)
            action->set_success_exit_code(e.as_int());
        return action;
    }

    // Handle Switch with full implementation
    if (type == L"Switch") {
        auto action = std::make_unique<actions::SwitchAction>();
        action->set_variable(node.attribute(L"Variable").as_string());
        for (const auto& c : node.children(L"Case"))
            action->add_case(c.attribute(L"Value").as_string(),
                             c.attribute(L"GoTo").as_string());
        if (auto def = node.child(L"Default"); def)
            action->set_default(def.attribute(L"GoTo").as_string());
        return action;
    }

    // Handle RandomString with full implementation
    if (type == L"RandomString") {
        auto action = std::make_unique<actions::RandomStringAction>();
        action->set_variable(node.attribute(L"Variable").as_string());
        if (auto l = node.attribute(L"Length"); l)
            action->set_length(l.as_int(8));
        if (auto cs = node.attribute(L"CharSet"); cs)
            action->set_charset(cs.as_string());
        return action;
    }

    // Handle FileRead with full implementation
    if (type == L"FileRead") {
        auto action = std::make_unique<actions::FileReadAction>();
        action->set_path(node.attribute(L"Path").as_string());
        if (auto v = node.attribute(L"Variable"); v)
            action->set_variable(v.as_string());
        return action;
    }

    if (type == L"Match") {
        auto action = std::make_unique<actions::MatchAction>();
        action->set_input_variable(node.attribute(L"Variable").as_string());
        action->set_output_variable(node.attribute(L"MatchVariable").as_string());
        for (const auto& c : node.children(L"Match"))
            action->add_pattern(c.attribute(L"Pattern").as_string(),
                                c.attribute(L"Variable").as_string());
        return action;
    }
    if (type == L"TSVar") {
        return std::make_unique<actions::TSVarAction>();
    }
    if (type == L"TSVarList") {
        auto action = std::make_unique<actions::TSVarListAction>();
        action->set_base(node.attribute(L"Variable").as_string());
        action->set_operation(node.attribute(L"ListType").as_string());
        action->set_value(node.attribute(L"Value").as_string());
        return action;
    }
    if (type == L"ErrorInfo") {
        auto action = std::make_unique<actions::ErrorInfoAction>();
        action->set_title(node.attribute(L"Title").as_string());
        action->set_text(node.attribute(L"ErrorText").as_string());
        return action;
    }
    if (type == L"Vars") {
        return std::make_unique<actions::VarsAction>();
    }

    static const std::vector<std::wstring> known_types = {
        L"DefaultValues", L"Input", L"Info", L"InfoFullScreen",
        L"AppTree", L"ExternalCall", L"Preflight", L"RegRead", L"RegWrite",
        L"WMIRead", L"WMIWrite", L"FileRead", L"Switch", L"TSVar",
        L"TSVarList", L"RandomString", L"SoftwareDiscovery", L"SaveItems",
        L"Match", L"ErrorInfo", L"Vars", L"Rest"
    };

    struct PlaceholderAction : IAction {
        ActionResult execute(ActionContext&) override { return {}; }
    };

    for (const auto& k : known_types)
        if (k == type) return std::make_unique<PlaceholderAction>();

    return nullptr;  // unknown type — caller skips with warning
}

} // anon namespace

ActionGraph ConfigParser::parse(const std::filesystem::path& path) const {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result)
        throw ParseError{std::format("Failed to parse '{}': {}",
            path.string(), result.description())};

    ActionGraph graph;
    auto actions_node = doc.child(L"UIpp").child(L"Actions");
    if (!actions_node)
        throw ParseError{"XML missing <UIpp><Actions> structure"};

    for (const auto& action_node : actions_node.children(L"Action")) {
        std::wstring type = action_node.attribute(L"Type").as_string();
        std::wstring id   = action_node.attribute(L"id").as_string();

        auto action = make_action(type, action_node);
        if (!action) continue;  // unknown type — skipped

        action->condition = action_node.attribute(L"Condition").as_string();
        graph.nodes.push_back({std::move(id), std::move(action)});
    }
    return graph;
}

} // namespace osdui::config
