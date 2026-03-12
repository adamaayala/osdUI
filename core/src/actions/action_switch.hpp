#pragma once
#include <osdui/action_graph.hpp>
#include <vector>
#include <utility>

namespace osdui::actions {

class SwitchAction : public IAction {
public:
    void set_variable(std::wstring var)                       { variable_ = std::move(var); }
    void add_case(std::wstring value, std::wstring goto_id)  { cases_.emplace_back(std::move(value), std::move(goto_id)); }
    void set_default(std::wstring goto_id)                   { default_ = std::move(goto_id); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring variable_;
    std::vector<std::pair<std::wstring, std::wstring>> cases_;
    std::wstring default_;
};

} // namespace osdui::actions
