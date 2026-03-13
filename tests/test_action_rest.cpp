#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_rest.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"
#include "mocks/mock_http_client.hpp"

using namespace osdui;
using namespace osdui::actions;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("REST: GET extracts JSON pointer value into variable") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    MockHttpClient http;
    http.enqueue({200, R"({"data": {"value": "DESKTOP-01"}})"});

    actions::RestAction action{http};
    action.set_url("https://api.example.com/osd");
    action.set_method(L"GET");
    action.set_variable(L"OSDResult");
    action.set_json_path(L"/data/value");
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDResult") == L"DESKTOP-01");
}

TEST_CASE("REST: non-2xx writes status code to variable") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    MockHttpClient http;
    http.enqueue({404, "Not Found"});

    actions::RestAction action{http};
    action.set_url("https://api.example.com/osd");
    action.set_variable(L"OSDResult");
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDResult") == L"404");
}

TEST_CASE("REST: network error writes '0' to variable") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    // A mock that throws HttpError on get()
    struct ThrowingHttpClient : http::IHttpClient {
        http::HttpResponse get(const std::string&,
                               const std::map<std::string,std::string>&) override {
            throw http::HttpError{"network failure"};
        }
        http::HttpResponse post(const std::string&, const std::string&,
                                const std::map<std::string,std::string>&) override {
            throw http::HttpError{"network failure"};
        }
    };

    ThrowingHttpClient http;
    actions::RestAction action{http};
    action.set_url("https://api.example.com/osd");
    action.set_variable(L"OSDResult");
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDResult") == L"0");
}
