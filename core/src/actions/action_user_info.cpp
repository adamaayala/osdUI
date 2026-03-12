#include "action_user_info.hpp"

namespace osdui::actions {

ActionResult UserInfoAction::execute(ActionContext& ctx) {
    model::DialogSpec spec{
        .title  = title_,
        .inputs = { model::InputSpec{.label = message_, .type = model::InputType::Info} },
    };
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

ActionResult InfoFullScreenAction::execute(ActionContext& ctx) {
    model::DialogSpec spec{
        .title       = title_,
        .banner_text = message_,
    };
    ctx.dialogs.present(spec, ctx.vars);
    return {};
}

} // namespace osdui::actions
