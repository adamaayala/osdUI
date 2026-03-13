#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_switch.hpp"
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

TEST_CASE("Switch: jumps to matching case") {
    MapVariableStore vars;
    vars.set(L"OSDModel", L"HP EliteBook");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::SwitchAction action;
    action.set_variable(L"OSDModel");
    action.add_case(L"HP EliteBook", L"hp_step");
    action.set_default(L"generic_step");

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::JumpTo);
    REQUIRE(result.jump_target == L"hp_step");
}

TEST_CASE("Switch: falls through to Default when no match") {
    MapVariableStore vars;
    vars.set(L"OSDModel", L"Unknown");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::SwitchAction action;
    action.set_variable(L"OSDModel");
    action.add_case(L"HP EliteBook", L"hp_step");
    action.set_default(L"generic_step");

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::JumpTo);
    REQUIRE(result.jump_target == L"generic_step");
}

TEST_CASE("Switch: returns Continue when no match and no Default") {
    MapVariableStore vars;
    vars.set(L"OSDModel", L"Unknown");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::SwitchAction action;
    action.set_variable(L"OSDModel");
    action.add_case(L"HP EliteBook", L"hp_step");
    // no default

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::Continue);
}

TEST_CASE("Switch: returns JumpTo with target id for runner to resolve") {
    MapVariableStore vars;
    vars.set(L"OSDModel", L"HP");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::SwitchAction action;
    action.set_variable(L"OSDModel");
    action.add_case(L"HP", L"nonexistent_id");

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::JumpTo);
    REQUIRE(result.jump_target == L"nonexistent_id");
}
