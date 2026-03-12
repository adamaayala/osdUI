#include "action_wmi_write.hpp"

namespace osdui::actions {

ActionResult WmiWriteAction::execute(ActionContext& ctx) {
    (void)ctx;
    wmi_.set(query_, property_, value_);
    return {};
}

} // namespace osdui::actions
