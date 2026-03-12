#include "action_reg_read.hpp"

namespace osdui::actions {

ActionResult RegReadAction::execute(ActionContext& ctx) {
    auto data = reg_.read(hive_, key_, value_);
    if (data && !variable_.empty())
        ctx.vars.set(variable_, *data);
    return {};
}

} // namespace osdui::actions
