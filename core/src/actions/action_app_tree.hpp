#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/model.hpp>
#include <vector>

namespace osdui::actions {

class AppTreeAction : public IAction {
public:
    void set_title(std::wstring title)            { title_ = std::move(title); }
    void add_software(model::SoftwareItem item)   { items_.push_back(std::move(item)); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring title_;
    std::vector<model::SoftwareItem> items_;
};

} // namespace osdui::actions
