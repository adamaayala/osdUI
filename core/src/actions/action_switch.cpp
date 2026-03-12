#include "action_switch.hpp"

namespace osdui::actions {

ActionResult SwitchAction::execute(ActionContext& ctx) {
    auto val = ctx.vars.get(variable_);
    std::wstring current = val ? *val : std::wstring{};

    for (const auto& [match, goto_id] : cases_) {
        if (current == match)
            return {ActionOutcome::JumpTo, goto_id};
    }

    if (!default_.empty())
        return {ActionOutcome::JumpTo, default_};

    return {};
}

} // namespace osdui::actions
