#include "action_external_call.hpp"
#include <format>

namespace osdui::actions {

ActionResult ExternalCallAction::execute(ActionContext& ctx) {
    auto result = ctx.scripts.execute(L"exe", run_);
    if (!variable_.empty())
        ctx.vars.set(variable_, std::format(L"{}", result.exit_code));
    return {};
}

} // namespace osdui::actions
