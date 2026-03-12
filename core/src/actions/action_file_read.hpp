#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

class FileReadAction : public IAction {
public:
    void set_path(std::wstring path)      { path_ = std::move(path); }
    void set_variable(std::wstring var)   { variable_ = std::move(var); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring path_;
    std::wstring variable_;
};

} // namespace osdui::actions
