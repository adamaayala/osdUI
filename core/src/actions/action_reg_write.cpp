#include "action_reg_write.hpp"

namespace osdui::actions {

ActionResult RegWriteAction::execute(ActionContext& ctx) {
    (void)ctx;
    reg_.write(hive_, key_, value_, data_);
    return {};
}

} // namespace osdui::actions
