#include "action_ts_var.hpp"
#include <osdui/model.hpp>

namespace osdui::actions {

ActionResult TSVarAction::execute(ActionContext& ctx) {
    if (is_setter_) {
        ctx.vars.set(variable_, value_);
        return {};
    }
    model::DialogSpec spec;
    spec.type  = model::DialogType::TsVar;
    spec.title = L"Task Sequence Variables";
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

} // namespace osdui::actions
