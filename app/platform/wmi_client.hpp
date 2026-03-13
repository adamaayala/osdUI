#pragma once
#include <osdui/iwmi.hpp>

namespace osdui::platform {

// IWmi backed by COM WMI (IWbemLocator / IWbemServices).
// COM must be initialized by the caller before constructing this object.
class WmiClient : public IWmi {
public:
    std::optional<std::wstring> query(std::wstring_view wql,
                                      std::wstring_view property) const override;
    void set(std::wstring_view wql, std::wstring_view property,
             std::wstring_view value) override;
};

} // namespace osdui::platform
