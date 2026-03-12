#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/model.hpp>

namespace osdui::actions {

class UserInfoAction : public IAction {
public:
    void set_title(std::wstring title)    { title_ = std::move(title); }
    void set_message(std::wstring msg)    { message_ = std::move(msg); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring title_;
    std::wstring message_;
};

class InfoFullScreenAction : public IAction {
public:
    void set_title(std::wstring title)    { title_ = std::move(title); }
    void set_message(std::wstring msg)    { message_ = std::move(msg); }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::wstring title_;
    std::wstring message_;
};

} // namespace osdui::actions
