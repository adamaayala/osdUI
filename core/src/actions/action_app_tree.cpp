#include "action_app_tree.hpp"

namespace osdui::actions {

ActionResult AppTreeAction::execute(ActionContext& ctx) {
    model::DialogSpec spec;
    spec.type   = model::DialogType::AppTree;
    spec.title  = title_;
    spec.groups = groups_;
    auto result = ctx.dialogs.present(spec, ctx.vars);
    if (!result.accepted) return {ActionOutcome::Abort};
    // Write selected software back to TS variables
    for (const auto& [key, value] : result.values)
        ctx.vars.set(key, value);
    return {};
}

} // namespace osdui::actions
