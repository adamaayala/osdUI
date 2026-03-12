#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/iregistry.hpp>

namespace osdui::actions {

class RegReadAction : public IAction {
public:
    explicit RegReadAction(IRegistry& reg) : reg_{reg} {}
    void set_hive(HKEY hive)               { hive_ = hive; }
    void set_key(std::wstring key)         { key_ = std::move(key); }
    void set_value(std::wstring value)     { value_ = std::move(value); }
    void set_variable(std::wstring var)    { variable_ = std::move(var); }
    ActionResult execute(ActionContext& ctx) override;
private:
    IRegistry&   reg_;
    HKEY         hive_{HKEY_LOCAL_MACHINE};
    std::wstring key_;
    std::wstring value_;
    std::wstring variable_;
};

} // namespace osdui::actions
