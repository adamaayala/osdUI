#pragma once
#include <osdui/interfaces.hpp>

namespace osdui::script {

// Evaluates boolean expressions used in condition= attributes.
// Supports: empty (true), "true"/"false"/"1"/"0",
//           %VAR% == "value", %VAR% != "value"
// Unknown/complex expressions: returns true (permissive) and logs a warning.
class ConditionEvaluator : public IConditionEvaluator {
public:
    bool evaluate(std::wstring_view expression,
                  const IVariableStore& vars) const override;

private:
    std::wstring substitute_vars(std::wstring_view expr,
                                 const IVariableStore& vars) const;
};

} // namespace osdui::script
