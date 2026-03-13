#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_match.hpp"
#include "../../core/src/actions/action_ts_var_list.hpp"
#include "../../core/src/actions/action_ts_var.hpp"
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

// ── Match tests ──────────────────────────────────────────────────────────────

TEST_CASE("Match: sets output var when pattern matches") {
    MapVariableStore vars;
    vars.set(L"OSDModel", L"HP EliteBook 840");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::MatchAction action;
    action.set_input_variable(L"OSDModel");
    action.set_output_variable(L"OSDMatchResult");
    action.add_pattern(L"HP*", L"HP");
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDMatchResult") == L"HP");
}

TEST_CASE("Match: leaves output var unset when no pattern matches") {
    MapVariableStore vars;
    vars.set(L"OSDModel", L"Unknown Device");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::MatchAction action;
    action.set_input_variable(L"OSDModel");
    action.set_output_variable(L"OSDMatchResult");
    action.add_pattern(L"HP*", L"HP");
    action.execute(ctx);

    REQUIRE_FALSE(vars.get(L"OSDMatchResult").has_value());
}

// ── TSVarList tests ──────────────────────────────────────────────────────────

TEST_CASE("TSVarList: append increments count and writes numbered entry") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::TSVarListAction action;
    action.set_base(L"OSDAppList");
    action.set_operation(L"Append");
    action.set_value(L"Notepad");
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDAppList") == L"1");
    REQUIRE(vars.get(L"OSDAppList001") == L"Notepad");
}

TEST_CASE("TSVarList: second append uses count 2") {
    MapVariableStore vars;
    vars.set(L"OSDAppList", L"1");
    vars.set(L"OSDAppList001", L"Notepad");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::TSVarListAction action;
    action.set_base(L"OSDAppList");
    action.set_operation(L"Append");
    action.set_value(L"7-Zip");
    action.execute(ctx);

    REQUIRE(vars.get(L"OSDAppList") == L"2");
    REQUIRE(vars.get(L"OSDAppList002") == L"7-Zip");
}

// ── TSVar tests ───────────────────────────────────────────────────────────────

TEST_CASE("TSVar: sets variable when Variable attribute present") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    TSVarAction action;
    action.set_variable_and_value(L"MyVar", L"42");
    action.execute(ctx);

    REQUIRE(vars.get(L"MyVar") == L"42");
    REQUIRE(dlg.calls_made() == 0);  // dialog must NOT have been shown
}

TEST_CASE("TSVar: shows dialog when no Variable attribute") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue({});   // must enqueue so mock doesn't throw
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    TSVarAction action;  // no set_variable_and_value() call — viewer mode
    action.execute(ctx);

    REQUIRE(dlg.calls_made() == 1);
    REQUIRE(dlg.last_spec().type == model::DialogType::TsVar);
}
