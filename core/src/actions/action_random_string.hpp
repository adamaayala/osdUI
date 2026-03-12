#pragma once
#include <osdui/action_graph.hpp>

namespace osdui::actions {

class RandomStringAction : public IAction {
public:
    void set_variable(std::wstring var)     { variable_ = std::move(var); }
    void set_length(int length)             { length_ = length; }
    void set_charset(std::wstring charset)  { charset_ = std::move(charset); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring variable_;
    int          length_{8};
    std::wstring charset_{L"alphanumeric"};
};

} // namespace osdui::actions
