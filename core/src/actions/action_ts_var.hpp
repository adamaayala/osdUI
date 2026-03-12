#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

// Displays a TS variable debug dialog.
class TSVarAction : public IAction {
public:
    ActionResult execute(ActionContext& ctx) override;
};

} // namespace osdui::actions
