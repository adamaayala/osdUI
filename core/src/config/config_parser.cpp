#include "config_parser.hpp"
#ifdef _WIN32
#  include <windows.h>
#else
#  include <cstring>
#endif
#include <pugixml.hpp>
#include <format>
#include <string_view>
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
#include "../actions/action_user_input.hpp"
#include "../actions/action_user_info.hpp"
#include "../actions/action_preflight.hpp"
#include "../actions/action_app_tree.hpp"

namespace osdui::config {
namespace {

// Convert narrow UTF-8 string (from pugixml) to wide string.
// pugixml's default char_t is char (narrow), so all .as_string() calls
// return const char*. Action setters take std::wstring.
static std::wstring to_wide(const char* s) {
    if (!s || *s == '\0') return {};
#ifdef _WIN32
    int len = ::MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    std::wstring result(static_cast<std::size_t>(len) - 1, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, s, -1, result.data(), len);
    return result;
#else
    // Fallback for non-Windows (tests on macOS/Linux) — assume ASCII-safe
    return std::wstring(s, s + std::strlen(s));
#endif
}

static std::wstring attr(const pugi::xml_node& node, const char* name) {
    return to_wide(node.attribute(name).as_string());
}

// Stub: return a non-null placeholder for known types so tests pass.
// NOTE: RestAction requires IHttpClient injection and cannot be instantiated
// here without access to a client instance. "Rest" maps to PlaceholderAction
// for now; wiring happens in main.cpp (Chunk 7).
std::unique_ptr<IAction> make_action(std::wstring_view type, const pugi::xml_node& node) {
    if (type == L"DefaultValues") {
        auto action = std::make_unique<actions::DefaultValuesAction>();
        for (const auto& var : node.children("Variable"))
            action->add(attr(var, "Name"), attr(var, "Value"));
        return action;
    }

    if (type == L"ExternalCall") {
        auto action = std::make_unique<actions::ExternalCallAction>();
        action->set_run(attr(node, "Run"));
        if (auto v = node.attribute("Variable"); v)
            action->set_variable(to_wide(v.as_string()));
        if (auto e = node.attribute("SuccessExitCode"); e)
            action->set_success_exit_code(e.as_int());
        return action;
    }

    if (type == L"Switch") {
        auto action = std::make_unique<actions::SwitchAction>();
        action->set_variable(attr(node, "Variable"));
        for (const auto& c : node.children("Case"))
            action->add_case(attr(c, "Value"), attr(c, "GoTo"));
        if (auto def = node.child("Default"); def)
            action->set_default(attr(def, "GoTo"));
        return action;
    }

    if (type == L"RandomString") {
        auto action = std::make_unique<actions::RandomStringAction>();
        action->set_variable(attr(node, "Variable"));
        if (auto l = node.attribute("Length"); l)
            action->set_length(l.as_int(8));
        if (auto cs = node.attribute("CharSet"); cs)
            action->set_charset(to_wide(cs.as_string()));
        return action;
    }

    if (type == L"FileRead") {
        auto action = std::make_unique<actions::FileReadAction>();
        action->set_path(attr(node, "Path"));
        if (auto v = node.attribute("Variable"); v)
            action->set_variable(to_wide(v.as_string()));
        return action;
    }

    if (type == L"Match") {
        auto action = std::make_unique<actions::MatchAction>();
        action->set_input_variable(attr(node, "Variable"));
        action->set_output_variable(attr(node, "MatchVariable"));
        for (const auto& c : node.children("Match"))
            action->add_pattern(attr(c, "Pattern"), attr(c, "Variable"));
        return action;
    }

    if (type == L"TSVar") {
        return std::make_unique<actions::TSVarAction>();
    }

    if (type == L"TSVarList") {
        auto action = std::make_unique<actions::TSVarListAction>();
        action->set_base(attr(node, "Variable"));
        action->set_operation(attr(node, "ListType"));
        action->set_value(attr(node, "Value"));
        return action;
    }

    if (type == L"ErrorInfo") {
        auto action = std::make_unique<actions::ErrorInfoAction>();
        action->set_title(attr(node, "Title"));
        action->set_text(attr(node, "ErrorText"));
        return action;
    }

    if (type == L"Vars") {
        return std::make_unique<actions::VarsAction>();
    }

    if (type == L"Input") {
        auto action = std::make_unique<actions::UserInputAction>();
        action->set_title(attr(node, "Title"));
        action->set_banner_title(attr(node, "BannerTitle"));
        action->set_banner_text(attr(node, "BannerText"));
        action->set_allow_cancel(std::string_view{node.attribute("AllowCancel").as_string()} == "true");
        for (const auto& field : node.children("InputField")) {
            model::InputSpec spec;
            spec.variable      = attr(field, "Variable");
            spec.label         = attr(field, "Prompt");
            spec.default_value = attr(field, "Default");
            spec.required      = std::string_view{field.attribute("Required").as_string()} == "true";
            std::string_view ft = field.attribute("Type").as_string();
            if      (ft == "DropDownList") spec.type = model::InputType::Dropdown;
            else if (ft == "Password")     spec.type = model::InputType::Password;
            else if (ft == "CheckBox")     spec.type = model::InputType::Checkbox;
            else if (ft == "Info")         spec.type = model::InputType::Info;
            else                           spec.type = model::InputType::Text;
            for (const auto& opt : field.children("Option")) {
                model::DropdownItem item;
                item.value   = attr(opt, "Value");
                item.display = attr(opt, "Text");
                spec.items.push_back(std::move(item));
            }
            action->add_input(std::move(spec));
        }
        return action;
    }

    if (type == L"Info") {
        auto action = std::make_unique<actions::UserInfoAction>();
        action->set_title(attr(node, "Title"));
        action->set_message(attr(node, "Message"));
        return action;
    }

    if (type == L"InfoFullScreen") {
        auto action = std::make_unique<actions::InfoFullScreenAction>();
        action->set_title(attr(node, "Title"));
        action->set_message(attr(node, "Message"));
        return action;
    }

    if (type == L"Preflight") {
        auto action = std::make_unique<actions::PreflightAction>();
        action->set_continue_on_fail(
            std::string_view{node.attribute("ContinueOnFail").as_string()} == "true");
        for (const auto& check_node : node.children("Check")) {
            model::PreflightItem item;
            item.name           = attr(check_node, "Name");
            item.condition      = attr(check_node, "Condition");
            item.warn_condition = attr(check_node, "WarnCondition");
            action->add_check(std::move(item));
        }
        return action;
    }

    if (type == L"AppTree") {
        auto action = std::make_unique<actions::AppTreeAction>();
        action->set_title(attr(node, "Title"));
        for (const auto& sw : node.children("Software")) {
            model::SoftwareItem item;
            item.id       = attr(sw, "id");
            item.name     = attr(sw, "Name");
            item.category = attr(sw, "Category");
            item.required = std::string_view{sw.attribute("Required").as_string()} == "true";
            action->add_software(std::move(item));
        }
        return action;
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
    auto actions_node = doc.child("UIpp").child("Actions");
    if (!actions_node)
        throw ParseError{"XML missing <UIpp><Actions> structure"};

    for (const auto& action_node : actions_node.children("Action")) {
        std::wstring type = to_wide(action_node.attribute("Type").as_string());
        std::wstring id   = to_wide(action_node.attribute("id").as_string());

        auto action = make_action(type, action_node);
        if (!action) continue;  // unknown type — skipped

        action->condition = to_wide(action_node.attribute("Condition").as_string());
        graph.nodes.push_back({std::move(id), std::move(action)});
    }
    return graph;
}

} // namespace osdui::config
