#pragma once
#include <osdui/interfaces.hpp>
#include <map>

namespace osdui::test {

// Returns pre-configured bool for each expression.
// Throws std::logic_error for any expression not explicitly configured —
// this makes test omissions visible rather than silently passing.
class LiteralConditionEvaluator : public IConditionEvaluator {
public:
    void set(std::wstring_view expression, bool value) {
        table_[std::wstring{expression}] = value;
    }
    bool evaluate(std::wstring_view expression,
                  const IVariableStore&) const override {
        if (expression.empty()) return true;
        auto it = table_.find(std::wstring{expression});
        if (it == table_.end())
            throw std::logic_error{"LiteralConditionEvaluator: unconfigured expression"};
        return it->second;
    }
private:
    std::map<std::wstring, bool> table_;
};

} // namespace osdui::test
