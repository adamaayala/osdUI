#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_app_tree.hpp"
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

TEST_CASE("AppTree: calls present and returns Continue when accepted") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue(model::DialogResult{true, {}});
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::AppTreeAction action;
    action.set_title(L"Select Software");
    model::SoftwareItem item;
    item.id   = L"SW001";
    item.name = L"Office Suite";
    action.add_software(std::move(item));

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
}

TEST_CASE("AppTree: result values are written to variable store") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue(model::DialogResult{true, {{L"SW001", L"true"}, {L"SW002", L"false"}}});
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::AppTreeAction action;
    action.set_title(L"Select Software");

    model::SoftwareItem item1;
    item1.id   = L"SW001";
    item1.name = L"Office Suite";
    action.add_software(std::move(item1));

    model::SoftwareItem item2;
    item2.id   = L"SW002";
    item2.name = L"Dev Tools";
    action.add_software(std::move(item2));

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
    REQUIRE(vars.has(L"SW001", L"true"));
    REQUIRE(vars.has(L"SW002", L"false"));
}
