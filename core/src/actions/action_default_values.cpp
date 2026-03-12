#include "action_default_values.hpp"

namespace osdui::actions {

ActionResult DefaultValuesAction::execute(ActionContext& ctx) {
    for (const auto& [name, value] : defaults_) {
        if (!ctx.vars.get(name).has_value())
            ctx.vars.set(name, value);
    }
    return {};
}

} // namespace osdui::actions
