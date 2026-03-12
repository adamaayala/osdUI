#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_file_read.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"
#include <fstream>
#include <filesystem>

using namespace osdui;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("FileRead: reads file contents into variable") {
    auto tmp = std::filesystem::temp_directory_path() / L"osdui_test_file_read.txt";
    { std::ofstream f{tmp}; f << "hello"; }

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::FileReadAction action;
    action.set_path(tmp.wstring());
    action.set_variable(L"OSDConfig");
    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
    REQUIRE(vars.get(L"OSDConfig") == L"hello");
    std::filesystem::remove(tmp);
}

TEST_CASE("FileRead: missing file leaves variable unset and returns Continue") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::FileReadAction action;
    action.set_path(L"C:\\definitely_does_not_exist_osdui.txt");
    action.set_variable(L"OSDConfig");
    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
    REQUIRE_FALSE(vars.get(L"OSDConfig").has_value());
}

TEST_CASE("FileRead: empty path leaves variable unset and returns Continue") {
    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::FileReadAction action;
    action.set_path(L"");
    action.set_variable(L"OSDConfig");
    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
    REQUIRE_FALSE(vars.get(L"OSDConfig").has_value());
}
