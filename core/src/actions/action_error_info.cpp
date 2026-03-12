#include "action_error_info.hpp"

namespace osdui::actions {

ActionResult ErrorInfoAction::execute(ActionContext& ctx) {
    model::DialogSpec spec;
    spec.title = title_;
    model::InputSpec info;
    info.type  = model::InputType::Info;
    info.label = text_;
    spec.inputs.push_back(std::move(info));
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

} // namespace osdui::actions
