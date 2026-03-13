#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/model.hpp>
#include <vector>

namespace osdui::actions {

class AppTreeAction : public IAction {
public:
    void set_title(std::wstring title)          { title_ = std::move(title); }
    void add_group(model::SoftwareGroup group)  { groups_.push_back(std::move(group)); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring title_;
    std::vector<model::SoftwareGroup> groups_;
};

} // namespace osdui::actions
