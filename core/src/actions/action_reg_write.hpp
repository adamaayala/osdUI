#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/iregistry.hpp>

namespace osdui::actions {

class RegWriteAction : public IAction {
public:
    explicit RegWriteAction(IRegistry& reg) : reg_{reg} {}
    void set_hive(HKEY hive)               { hive_ = hive; }
    void set_key(std::wstring key)         { key_ = std::move(key); }
    void set_value(std::wstring value)     { value_ = std::move(value); }
    void set_data(std::wstring data)       { data_ = std::move(data); }
    ActionResult execute(ActionContext& ctx) override;
private:
    IRegistry&   reg_;
    HKEY         hive_{HKEY_LOCAL_MACHINE};
    std::wstring key_;
    std::wstring value_;
    std::wstring data_;
};

} // namespace osdui::actions
