#include "action_match.hpp"

namespace osdui::actions {

bool MatchAction::matches(std::wstring_view pattern, std::wstring_view value) {
    if (pattern.empty()) return false;
    bool prefix_wild = pattern.front() == L'*';
    bool suffix_wild = pattern.back()  == L'*';
    std::wstring_view core = pattern;
    if (prefix_wild) core.remove_prefix(1);
    if (suffix_wild && !core.empty()) core.remove_suffix(1);

    if (prefix_wild && suffix_wild) return value.find(core) != std::wstring_view::npos;
    if (prefix_wild)                return value.size() >= core.size() &&
                                           value.substr(value.size() - core.size()) == core;
    if (suffix_wild)                return value.substr(0, core.size()) == core;
    return value == core;
}

ActionResult MatchAction::execute(ActionContext& ctx) {
    auto val = ctx.vars.get(input_variable_);
    if (!val) return {};

    for (const auto& [pattern, result] : patterns_) {
        if (matches(pattern, *val)) {
            if (!output_variable_.empty())
                ctx.vars.set(output_variable_, result);
            return {};
        }
    }
    return {};
}

} // namespace osdui::actions
