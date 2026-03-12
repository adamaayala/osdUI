#include "action_wmi_read.hpp"

namespace osdui::actions {

ActionResult WmiReadAction::execute(ActionContext& ctx) {
    auto result = wmi_.query(query_, property_);
    if (result && !variable_.empty())
        ctx.vars.set(variable_, *result);
    return {};
}

} // namespace osdui::actions
