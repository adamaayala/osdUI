#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

class TSVarListAction : public IAction {
public:
    void set_base(std::wstring base)        { base_ = std::move(base); }
    void set_operation(std::wstring op)     { operation_ = std::move(op); }
    void set_value(std::wstring value)      { value_ = std::move(value); }
    void set_variable(std::wstring var)     { variable_ = std::move(var); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring base_;
    std::wstring operation_;
    std::wstring value_;
    std::wstring variable_;  // for Count operation: store count here
};

} // namespace osdui::actions
