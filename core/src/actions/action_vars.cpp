#include "action_vars.hpp"

namespace osdui::actions {

ActionResult VarsAction::execute(ActionContext& ctx) {
    model::DialogSpec spec;
    spec.title = L"Variables";
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

} // namespace osdui::actions
