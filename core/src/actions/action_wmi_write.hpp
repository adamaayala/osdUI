#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/iwmi.hpp>

namespace osdui::actions {

class WmiWriteAction : public IAction {
public:
    explicit WmiWriteAction(IWmi& wmi) : wmi_{wmi} {}
    void set_query(std::wstring query)      { query_ = std::move(query); }
    void set_property(std::wstring prop)    { property_ = std::move(prop); }
    void set_value(std::wstring value)      { value_ = std::move(value); }
    ActionResult execute(ActionContext& ctx) override;
private:
    IWmi&        wmi_;
    std::wstring query_;
    std::wstring property_;
    std::wstring value_;
};

} // namespace osdui::actions
