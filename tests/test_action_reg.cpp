#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_reg_read.hpp"
#include "../../core/src/actions/action_reg_write.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"
#include "mocks/mock_registry.hpp"

using namespace osdui;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("RegRead: reads value into TS variable") {
    MapRegistry reg;
    reg.set(HKEY_LOCAL_MACHINE, L"SOFTWARE\\OSD", L"Version", L"1.0");

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    RegReadAction action{reg};
    action.set_hive(HKEY_LOCAL_MACHINE);
    action.set_key(L"SOFTWARE\\OSD");
    action.set_value(L"Version");
    action.set_variable(L"OSDVersion");
    action.execute(ctx);

    REQUIRE(vars.has(L"OSDVersion", L"1.0"));
}

TEST_CASE("RegRead: missing key leaves variable unset") {
    MapRegistry reg;  // empty

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    RegReadAction action{reg};
    action.set_hive(HKEY_LOCAL_MACHINE);
    action.set_key(L"SOFTWARE\\OSD");
    action.set_value(L"Version");
    action.set_variable(L"OSDVersion");
    action.execute(ctx);

    REQUIRE_FALSE(vars.get(L"OSDVersion").has_value());
}

TEST_CASE("RegWrite: writes data to registry") {
    MapRegistry reg;

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    RegWriteAction action{reg};
    action.set_hive(HKEY_LOCAL_MACHINE);
    action.set_key(L"SOFTWARE\\OSD");
    action.set_value(L"Deployed");
    action.set_data(L"true");
    action.execute(ctx);

    REQUIRE(reg.read(HKEY_LOCAL_MACHINE, L"SOFTWARE\\OSD", L"Deployed") == L"true");
}
