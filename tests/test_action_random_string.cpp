#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_random_string.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"
#include <algorithm>
#include <cwctype>

using namespace osdui;
using namespace osdui::actions;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("RandomString: output has correct length") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::RandomStringAction action;
    action.set_variable(L"OSDPassword");
    action.set_length(12);
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDPassword").has_value());
    REQUIRE(vars.get(L"OSDPassword")->length() == 12);
}

TEST_CASE("RandomString: alphanumeric charset contains only [A-Za-z0-9]") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::RandomStringAction action;
    action.set_variable(L"OSDPassword");
    action.set_length(100);
    action.set_charset(L"alphanumeric");
    action.execute(ctx);

    auto val = vars.get(L"OSDPassword").value();
    REQUIRE(std::ranges::all_of(val, [](wchar_t c) {
        return std::iswalnum(c);
    }));
}

TEST_CASE("RandomString: numeric charset contains only [0-9]") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::RandomStringAction action;
    action.set_variable(L"OSDPassword");
    action.set_length(50);
    action.set_charset(L"numeric");
    action.execute(ctx);

    auto val = vars.get(L"OSDPassword").value();
    REQUIRE(std::ranges::all_of(val, [](wchar_t c) {
        return std::iswdigit(c);
    }));
}

TEST_CASE("RandomString: sets variable") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::RandomStringAction action;
    action.set_variable(L"OSDToken");
    action.set_length(8);
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDToken").has_value());
}
