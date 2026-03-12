#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_user_info.hpp"
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

TEST_CASE("UserInfo: calls present and returns Continue") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue(model::DialogResult{true, {}});
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::UserInfoAction action;
    action.set_title(L"Info Title");
    action.set_message(L"This is an informational message.");

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
}

TEST_CASE("InfoFullScreen: calls present and returns Continue") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue(model::DialogResult{true, {}});
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::InfoFullScreenAction action;
    action.set_title(L"Full Screen Title");
    action.set_message(L"Full screen banner message.");

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
}
