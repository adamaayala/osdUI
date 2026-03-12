#include "config_parser.hpp"
#include <pugixml.hpp>
#include <format>
#include <vector>

namespace osdui::config {
namespace {

// Stub: return a non-null placeholder for known types so tests pass.
// Real factories replace individual branches in Chunk 5+.
std::unique_ptr<IAction> make_action(std::wstring_view type, const pugi::xml_node& /*node*/) {
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
