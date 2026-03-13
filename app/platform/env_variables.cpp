#include "env_variables.hpp"
#include <windows.h>
#include <string>
#include <optional>

namespace osdui::platform {

std::optional<std::wstring> EnvVariables::get(std::wstring_view name) const {
    std::wstring name_buf{name};
    DWORD needed = GetEnvironmentVariableW(name_buf.c_str(), nullptr, 0);
    if (needed == 0) return std::nullopt;  // variable not set

    std::wstring value(needed - 1, L'\0');
    GetEnvironmentVariableW(name_buf.c_str(), value.data(), needed);
    return value;
}

void EnvVariables::set(std::wstring_view name, std::wstring_view value) {
    std::wstring name_buf{name};
    std::wstring value_buf{value};
    SetEnvironmentVariableW(name_buf.c_str(), value_buf.c_str());
}

} // namespace osdui::platform
