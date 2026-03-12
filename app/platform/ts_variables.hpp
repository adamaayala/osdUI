#pragma once
#include <osdui/interfaces.hpp>
#include <wil/com.h>

namespace osdui::platform {

// IVariableStore backed by the SCCM task sequence COM environment.
// Throws wil::ResultException if COM calls fail.
class TsVariables : public IVariableStore {
public:
    TsVariables();   // CoInitializes and binds ITSEnvClass
    ~TsVariables();

    std::optional<std::wstring> get(std::wstring_view name) const override;
    void set(std::wstring_view name, std::wstring_view value) override;

private:
    wil::com_ptr<IDispatch> ts_env_;
};

} // namespace osdui::platform
