#pragma once
#include <optional>
#include <string>

namespace osdui {

struct IWmi {
    virtual ~IWmi() = default;
    virtual std::optional<std::wstring> query(std::wstring_view wql,
                                               std::wstring_view property) const = 0;
    virtual void set(std::wstring_view wql, std::wstring_view property,
                     std::wstring_view value) = 0;
};

} // namespace osdui
