#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

class ErrorInfoAction : public IAction {
public:
    void set_title(std::wstring title) { title_ = std::move(title); }
    void set_text(std::wstring text)   { text_ = std::move(text); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring title_;
    std::wstring text_;
};

} // namespace osdui::actions
