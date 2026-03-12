#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/model.hpp>
#include <vector>

namespace osdui::actions {

class UserInputAction : public IAction {
public:
    void set_title(std::wstring title)        { spec_.title = std::move(title); }
    void set_banner_title(std::wstring t)     { spec_.banner_title = std::move(t); }
    void set_banner_text(std::wstring t)      { spec_.banner_text = std::move(t); }
    void set_allow_cancel(bool v)             { spec_.allow_cancel = v; }
    void add_input(model::InputSpec input)    { spec_.inputs.push_back(std::move(input)); }
    ActionResult execute(ActionContext& ctx) override;
private:
    model::DialogSpec spec_;
};

} // namespace osdui::actions
