#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/model.hpp>
#include <vector>

namespace osdui::actions {

class PreflightAction : public IAction {
public:
    void set_continue_on_fail(bool v)                { continue_on_fail_ = v; }
    void add_check(model::PreflightItem check)       { checks_.push_back(std::move(check)); }
    ActionResult execute(ActionContext& ctx) override;
private:
    bool continue_on_fail_{false};
    std::vector<model::PreflightItem> checks_;
};

} // namespace osdui::actions
