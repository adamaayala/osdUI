#include "action_user_input.hpp"

namespace osdui::actions {

ActionResult UserInputAction::execute(ActionContext& ctx) {
    auto result = ctx.dialogs.present(spec_, ctx.vars);
    if (!result.accepted) return {ActionOutcome::Abort};
    for (const auto& [key, value] : result.values)
        ctx.vars.set(key, value);
    return {};
}

} // namespace osdui::actions
