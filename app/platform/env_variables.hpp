#pragma once
#include <osdui/interfaces.hpp>

namespace osdui::platform {

// IVariableStore backed by Windows process environment variables.
// Used as a fallback when Microsoft.SMS.TSEnvironment is unavailable
// (i.e. running outside an active SCCM task sequence, e.g. for local testing).
class EnvVariables : public IVariableStore {
public:
    std::optional<std::wstring> get(std::wstring_view name) const override;
    void set(std::wstring_view name, std::wstring_view value) override;
};

} // namespace osdui::platform
