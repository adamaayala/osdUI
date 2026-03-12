#include "action_ts_var.hpp"

namespace osdui::actions {

ActionResult TSVarAction::execute(ActionContext& ctx) {
    model::DialogSpec spec;
    spec.title = L"Task Sequence Variables";
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

} // namespace osdui::actions
