#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_user_input.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"

using namespace osdui;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("UserInput: accepted dialog writes values to vars") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue(model::DialogResult{true, {{L"OSDComputerName", L"DESKTOP-01"}}});
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::UserInputAction action;
    action.set_title(L"System Info");
    model::InputSpec field;
    field.variable = L"OSDComputerName";
    field.label    = L"Computer Name";
    field.type     = model::InputType::Text;
    action.add_input(std::move(field));

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
    REQUIRE(vars.has(L"OSDComputerName", L"DESKTOP-01"));
}

TEST_CASE("UserInput: cancelled dialog returns Abort without writing vars") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue(model::DialogResult{false, {}});
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::UserInputAction action;
    action.set_title(L"System Info");

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Abort);
    REQUIRE_FALSE(vars.get(L"OSDComputerName").has_value());
}
