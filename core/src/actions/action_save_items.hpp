#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

// Writes selected software items to a file path.
class SaveItemsAction : public IAction {
public:
    void set_path(std::wstring path) { path_ = std::move(path); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring path_;
};

} // namespace osdui::actions
