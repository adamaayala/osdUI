#pragma once
#include <osdui/action_graph.hpp>
#include <string>

namespace osdui::actions {

class TSVarAction : public IAction {
public:
    // Call these when XML has Variable= attribute (set-variable mode).
    // Without calling set_variable(), execute() shows the viewer dialog.
    void set_variable(std::wstring var) { variable_ = std::move(var); is_setter_ = true; }
    void set_value(std::wstring val)    { value_ = std::move(val); }

    ActionResult execute(ActionContext& ctx) override;
private:
    bool         is_setter_{false};
    std::wstring variable_;
    std::wstring value_;
};

} // namespace osdui::actions
