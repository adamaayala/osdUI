#pragma once
#include <osdui/action_graph.hpp>
#include <string>

namespace osdui::actions {

class TSVarAction : public IAction {
public:
    // Call this when XML has Variable= attribute (set-variable mode).
    // Without calling set_variable_and_value(), execute() shows the viewer dialog.
    void set_variable_and_value(std::wstring var, std::wstring val) {
        variable_ = std::move(var);
        value_    = std::move(val);
        is_setter_ = true;
    }

    ActionResult execute(ActionContext& ctx) override;
private:
    bool         is_setter_{false};
    std::wstring variable_;
    std::wstring value_;
};

} // namespace osdui::actions
