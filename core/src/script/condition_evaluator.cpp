#include "condition_evaluator.hpp"
#include <algorithm>
#include <regex>

namespace osdui::script {

std::wstring ConditionEvaluator::substitute_vars(
    std::wstring_view expr, const IVariableStore& vars) const
{
    std::wstring result{expr};
    std::wregex var_re{LR"(%([^%]+)%)"};
    std::wstring out;
    auto it = std::wsregex_iterator(result.begin(), result.end(), var_re);
    auto end = std::wsregex_iterator{};
    std::size_t last = 0;
    for (; it != end; ++it) {
        const auto& m = *it;
        out += result.substr(last, m.position() - last);
        auto val = vars.get(m[1].str());
        out += val ? *val : std::wstring{};
        last = m.position() + m.length();
    }
    out += result.substr(last);
    return out;
}

bool ConditionEvaluator::evaluate(
    std::wstring_view expression, const IVariableStore& vars) const
{
    if (expression.empty()) return true;

    std::wstring expr = substitute_vars(expression, vars);

    // Trim whitespace
    auto trim = [](std::wstring& s) {
        s.erase(0, s.find_first_not_of(L" \t"));
        s.erase(s.find_last_not_of(L" \t") + 1);
    };
    trim(expr);

    // Literal
    if (expr == L"true"  || expr == L"1") return true;
    if (expr == L"false" || expr == L"0") return false;

    // Equality: lhs == "rhs" or lhs != "rhs"
    std::wregex eq_re {LR"(^(.+?)\s*(==|!=)\s*\"(.*?)\"$)"};
    std::wsmatch m;
    if (std::regex_match(expr, m, eq_re)) {
        std::wstring lhs = m[1].str(); trim(lhs);
        std::wstring op  = m[2].str();
        std::wstring rhs = m[3].str();
        bool eq = (lhs == rhs);
        return (op == L"==") ? eq : !eq;
    }

    // Permissive fallback for complex expressions
    return true;
}

} // namespace osdui::script
