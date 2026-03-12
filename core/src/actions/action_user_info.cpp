#include "action_user_info.hpp"

namespace osdui::actions {

ActionResult UserInfoAction::execute(ActionContext& ctx) {
    model::DialogSpec spec;
    spec.title = title_;
    model::InputSpec info;
    info.type  = model::InputType::Info;
    info.label = message_;
    spec.inputs.push_back(std::move(info));
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

ActionResult InfoFullScreenAction::execute(ActionContext& ctx) {
    model::DialogSpec spec;
    spec.title      = title_;
    spec.banner_text = message_;
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

} // namespace osdui::actions
