#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_app_tree.hpp"
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

TEST_CASE("AppTree: presents dialog with groups and writes results to vars") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue({true, {{L"Office2024", L"true"}}});
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    model::SoftwareItem item;
    item.id               = L"Office2024";
    item.name             = L"Office 2024";
    item.default_selected = true;

    model::SoftwareGroup grp;
    grp.id    = L"DefaultApps";
    grp.label = L"Default Applications";
    grp.items.push_back(item);

    AppTreeAction action;
    action.set_title(L"Select Software");
    action.add_group(std::move(grp));
    auto result = action.execute(ctx);

    REQUIRE(dlg.calls_made() == 1);
    REQUIRE(dlg.last_spec().groups.size() == 1);
    REQUIRE(dlg.last_spec().groups[0].items.size() == 1);
    REQUIRE(vars.get(L"Office2024") == L"true");
}

TEST_CASE("AppTree: cancelled dialog returns Abort") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue({false, {}});
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    AppTreeAction action;
    action.set_title(L"Select Software");
    action.add_group(model::SoftwareGroup{});
    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Abort);
}
