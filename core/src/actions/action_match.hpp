#pragma once
#include <osdui/action_graph.hpp>
#include <vector>
#include <utility>

namespace osdui::actions {

class MatchAction : public IAction {
public:
    void set_input_variable(std::wstring var)  { input_variable_ = std::move(var); }
    void set_output_variable(std::wstring var) { output_variable_ = std::move(var); }
    // pattern supports leading/trailing * wildcard
    void add_pattern(std::wstring pattern, std::wstring result) {
        patterns_.emplace_back(std::move(pattern), std::move(result));
    }
    ActionResult execute(ActionContext& ctx) override;
private:
    static bool matches(std::wstring_view pattern, std::wstring_view value);
    std::wstring input_variable_;
    std::wstring output_variable_;
    std::vector<std::pair<std::wstring, std::wstring>> patterns_;
};

} // namespace osdui::actions
