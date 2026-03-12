#pragma once
#include <optional>
#include <string>
#include <map>
#include "model.hpp"

namespace osdui {

struct IVariableStore {
    virtual ~IVariableStore() = default;
    virtual std::optional<std::wstring> get(std::wstring_view name) const = 0;
    // Throws wil::ResultException on COM failure in production impl
    virtual void set(std::wstring_view name, std::wstring_view value) = 0;
};

struct IDialogPresenter {
    virtual ~IDialogPresenter() = default;
    virtual model::DialogResult present(const model::DialogSpec& spec,
                                        const IVariableStore& vars) = 0;
};

struct IScriptHost {
    virtual ~IScriptHost() = default;
    // language: empty/"exe" → CreateProcess; "vbscript"/"jscript" → WSH
    virtual model::ScriptResult execute(std::wstring_view language,
                                        std::wstring_view script) = 0;
};

struct IConditionEvaluator {
    virtual ~IConditionEvaluator() = default;
    virtual bool evaluate(std::wstring_view expression,
                          const IVariableStore& vars) const = 0;
};

} // namespace osdui
