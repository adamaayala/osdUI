#pragma once
#include <optional>
#include <string>
#include <windows.h>

namespace osdui {

struct IRegistry {
    virtual ~IRegistry() = default;
    virtual std::optional<std::wstring> read(HKEY hive, std::wstring_view key,
                                              std::wstring_view value) const = 0;
    virtual void write(HKEY hive, std::wstring_view key,
                       std::wstring_view value, std::wstring_view data) = 0;
};

} // namespace osdui
