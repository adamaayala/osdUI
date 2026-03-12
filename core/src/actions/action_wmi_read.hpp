#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/iwmi.hpp>

namespace osdui::actions {

class WmiReadAction : public IAction {
public:
    explicit WmiReadAction(IWmi& wmi) : wmi_{wmi} {}
    void set_query(std::wstring query)      { query_ = std::move(query); }
    void set_property(std::wstring prop)    { property_ = std::move(prop); }
    void set_variable(std::wstring var)     { variable_ = std::move(var); }
    ActionResult execute(ActionContext& ctx) override;
private:
    IWmi&        wmi_;
    std::wstring query_;
    std::wstring property_;
    std::wstring variable_;
};

} // namespace osdui::actions
