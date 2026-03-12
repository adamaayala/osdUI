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

    // Build a dialog spec to show results.
    // TODO(chunk7): PreflightItem status (Pass/Warn/Fail) is not yet carried through
    // to DialogSpec because InputSpec has no status field. The Win32 DialogPresenter
    // in app/platform/win32_dialog_presenter.cpp will need a preflight-specific
    // overload or InputType::Preflight variant. See Task 25.
    model::DialogSpec spec{ .title = L"Preflight Checks" };
    for (const auto& check : evaluated) {
        spec.inputs.push_back(model::InputSpec{
            .label = check.name,
            .type  = model::InputType::Info,
        });
    }
    ctx.dialogs.present(spec, ctx.vars);

    if (any_fail && !continue_on_fail_)
        return {ActionOutcome::Abort};
    return {};
}

} // namespace osdui::actions
