#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/ihttp_client.hpp>
#include <string>

namespace osdui::actions {

class RestAction : public IAction {
public:
    explicit RestAction(http::IHttpClient& client) : client_{client} {}

    void set_url(std::string url)          { url_ = std::move(url); }
    void set_method(std::wstring method)   { method_ = std::move(method); }
    void set_variable(std::wstring var)    { variable_ = std::move(var); }
    void set_json_path(std::wstring path)  { json_path_ = std::move(path); }
    void set_body(std::string body)        { body_ = std::move(body); }

    ActionResult execute(ActionContext& ctx) override;

private:
    http::IHttpClient& client_;
    std::string  url_;
    std::wstring method_{L"GET"};
    std::wstring variable_;
    std::wstring json_path_;
    std::string  body_;
};

} // namespace osdui::actions
