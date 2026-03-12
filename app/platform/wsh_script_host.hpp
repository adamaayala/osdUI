#pragma once
#include <osdui/interfaces.hpp>

namespace osdui::platform {

// IScriptHost implementation:
//   language == L"exe" or empty → CreateProcessW
//   otherwise → IActiveScript / IActiveScriptParse (Windows Script Host)
// COM must be initialized by the caller before constructing this object.
class WshScriptHost : public IScriptHost {
public:
    WshScriptHost() = default;
    model::ScriptResult execute(std::wstring_view language,
                                std::wstring_view script) override;
};

} // namespace osdui::platform
