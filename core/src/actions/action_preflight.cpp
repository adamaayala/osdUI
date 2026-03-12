#include "action_preflight.hpp"

namespace osdui::actions {

ActionResult PreflightAction::execute(ActionContext& ctx) {
    bool any_fail = false;

    // Evaluate all check conditions
    std::vector<model::PreflightItem> evaluated = checks_;
    for (auto& check : evaluated) {
        bool pass = ctx.conditions.evaluate(check.condition, ctx.vars);
        if (!pass) {
            bool warn = !check.warn_condition.empty() &&
                        ctx.conditions.evaluate(check.warn_condition, ctx.vars);
            check.status = warn ? model::PreflightStatus::Warn
                                : model::PreflightStatus::Fail;
            if (check.status == model::PreflightStatus::Fail)
                any_fail = true;
        } else {
            check.status = model::PreflightStatus::Pass;
        }
    }

    // Build a dialog spec to show results
    model::DialogSpec spec;
    spec.title = L"Preflight Checks";
    for (const auto& check : evaluated) {
        model::InputSpec item;
        item.type  = model::InputType::Info;
        item.label = check.name;
        spec.inputs.push_back(std::move(item));
    }
    ctx.dialogs.present(spec, ctx.vars);

    if (any_fail && !continue_on_fail_)
        return {ActionOutcome::Abort};
    return {};
}

} // namespace osdui::actions
