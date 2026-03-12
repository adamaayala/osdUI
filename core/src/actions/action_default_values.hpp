#pragma once
#include <osdui/action_graph.hpp>
#include <vector>
#include <utility>

namespace osdui::actions {

class DefaultValuesAction : public IAction {
public:
    void add(std::wstring name, std::wstring value) {
        defaults_.emplace_back(std::move(name), std::move(value));
    }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::vector<std::pair<std::wstring, std::wstring>> defaults_;
};

} // namespace osdui::actions
