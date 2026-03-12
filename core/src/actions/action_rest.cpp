#include "action_rest.hpp"
#include <osdui/ihttp_client.hpp>
#include <nlohmann/json.hpp>
#include <format>

namespace osdui::actions {

namespace {

// Convert narrow UTF-8 string to wide string
std::wstring to_wstring(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

} // anon

ActionResult RestAction::execute(ActionContext& ctx) {
    if (variable_.empty()) return {};

    try {
        http::HttpResponse resp;
        if (method_ == L"POST")
            resp = client_.post(url_, body_);
        else
            resp = client_.get(url_);

        if (!resp.ok()) {
            ctx.vars.set(variable_, std::format(L"{}", resp.status_code));
            return {};
        }

        if (json_path_.empty()) {
            ctx.vars.set(variable_, to_wstring(resp.body));
            return {};
        }

        // Extract value via JSON Pointer
        try {
            auto doc = nlohmann::json::parse(resp.body);
            std::string path(json_path_.begin(), json_path_.end());
            auto val = doc.at(nlohmann::json::json_pointer(path));
            if (val.is_string())
                ctx.vars.set(variable_, to_wstring(val.get<std::string>()));
            else
                ctx.vars.set(variable_, to_wstring(val.dump()));
        } catch (const nlohmann::json::exception&) {
            ctx.vars.set(variable_, L"PARSE_ERROR");
        }

    } catch (const http::HttpError&) {
        ctx.vars.set(variable_, L"0");
    }

    return {};
}

} // namespace osdui::actions
