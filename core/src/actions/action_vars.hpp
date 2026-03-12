#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

// Displays all current TS variables via the dialog presenter (debug).
class VarsAction : public IAction {
public:
    ActionResult execute(ActionContext& ctx) override;
};

} // namespace osdui::actions
