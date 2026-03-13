#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_preflight.hpp"
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

TEST_CASE("Preflight: all checks pass returns Continue") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    // Preflight result is determined by check conditions, not dialog acceptance.
    // The `accepted` field of DialogResult is intentionally ignored.
    dlg.enqueue(model::DialogResult{true, {}});
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    ce.set(L"MemGBge4", true);
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::PreflightAction action;
    action.set_continue_on_fail(false);
    model::PreflightItem check;
    check.name      = L"Memory";
    check.condition = L"MemGBge4";
    action.add_check(std::move(check));

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::Continue);
}

TEST_CASE("Preflight: failed check with ContinueOnFail=false returns Abort") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    // Preflight result is determined by check conditions, not dialog acceptance.
    // The `accepted` field of DialogResult is intentionally ignored.
    dlg.enqueue(model::DialogResult{true, {}});
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    ce.set(L"MemGBge4", false);
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::PreflightAction action;
    action.set_continue_on_fail(false);
    model::PreflightItem check;
    check.name      = L"Memory";
    check.condition = L"MemGBge4";
    action.add_check(std::move(check));

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::Abort);
}

TEST_CASE("Preflight: failed check with ContinueOnFail=true returns Continue") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    // Preflight result is determined by check conditions, not dialog acceptance.
    // The `accepted` field of DialogResult is intentionally ignored.
    dlg.enqueue(model::DialogResult{true, {}});
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    ce.set(L"MemGBge4", false);
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::PreflightAction action;
    action.set_continue_on_fail(true);
    model::PreflightItem check;
    check.name      = L"Memory";
    check.condition = L"MemGBge4";
    action.add_check(std::move(check));

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::Continue);
}

TEST_CASE("Preflight: failed check with warn_condition true gets Warn status, returns Continue") {
    // Main condition fails, but warn_condition passes → item is Warn, not Fail.
    // Warn is not a hard fail: any_fail stays false, so the action returns
    // Continue even when continue_on_fail is false.
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    // Preflight result is determined by check conditions, not dialog acceptance.
    // The `accepted` field of DialogResult is intentionally ignored.
    dlg.enqueue(model::DialogResult{true, {}});
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    ce.set(L"DiskGBge100", false);     // main condition: fails
    ce.set(L"DiskGBge50",  true);      // warn_condition: passes → Warn, not Fail
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::PreflightAction action;
    action.set_continue_on_fail(false); // even with strict mode, Warn is not Abort
    model::PreflightItem check;
    check.name           = L"Disk Space";
    check.condition      = L"DiskGBge100";
    check.warn_condition = L"DiskGBge50";
    action.add_check(std::move(check));

    auto result = action.execute(ctx);
    REQUIRE(result.outcome == ActionOutcome::Continue);
}
