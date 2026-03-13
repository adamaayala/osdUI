#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_default_values.hpp"
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

TEST_CASE("DefaultValues: sets variables") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::DefaultValuesAction action;
    action.add(L"OSDComputerName", L"DESKTOP");
    action.add(L"OSDSiteCode",     L"P01");

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
    REQUIRE(vars.has(L"OSDComputerName", L"DESKTOP"));
    REQUIRE(vars.has(L"OSDSiteCode",     L"P01"));
}

TEST_CASE("DefaultValues: does not overwrite existing variable") {
    MapVariableStore vars;
    vars.set(L"OSDComputerName", L"EXISTING");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::DefaultValuesAction action;
    action.add(L"OSDComputerName", L"DEFAULT");

    action.execute(ctx);
    REQUIRE(vars.has(L"OSDComputerName", L"EXISTING"));
}
