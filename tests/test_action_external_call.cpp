#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_external_call.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"

using namespace osdui;
using namespace osdui::actions;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("ExternalCall: invokes script host with language 'exe'") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    sh.enqueue(0);
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::ExternalCallAction action;
    action.set_run(L"cmd.exe /c echo hello");
    action.execute(ctx);

    REQUIRE(sh.invocations().size() == 1);
    REQUIRE(sh.invocations()[0].language == L"exe");
    REQUIRE(sh.invocations()[0].script   == L"cmd.exe /c echo hello");
}

TEST_CASE("ExternalCall: writes exit code to variable as string") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    sh.enqueue(42);
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::ExternalCallAction action;
    action.set_run(L"something.exe");
    action.set_variable(L"OSDResult");
    action.execute(ctx);

    REQUIRE(vars.has(L"OSDResult", L"42"));
}
