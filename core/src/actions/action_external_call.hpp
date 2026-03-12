#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

class ExternalCallAction : public IAction {
public:
    void set_run(std::wstring cmd)      { run_ = std::move(cmd); }
    void set_variable(std::wstring var) { variable_ = std::move(var); }
    void set_success_exit_code(int c)   { success_exit_code_ = c; }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring run_;
    std::wstring variable_;
    int          success_exit_code_{0};
};

} // namespace osdui::actions
