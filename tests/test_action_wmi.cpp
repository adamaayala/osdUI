#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_wmi_read.hpp"
#include "../../core/src/actions/action_wmi_write.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"
#include "mocks/mock_wmi.hpp"

using namespace osdui;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("WMIRead: sets TS variable from query result") {
    MapWmi wmi;
    wmi.set_result(L"SELECT Manufacturer FROM Win32_ComputerSystem", L"Manufacturer", L"HP");

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    WmiReadAction action{wmi};
    action.set_query(L"SELECT Manufacturer FROM Win32_ComputerSystem");
    action.set_property(L"Manufacturer");
    action.set_variable(L"OSDManufacturer");
    action.execute(ctx);

    REQUIRE(vars.has(L"OSDManufacturer", L"HP"));
}

TEST_CASE("WMIRead: missing result leaves variable unset") {
    MapWmi wmi;  // empty

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    WmiReadAction action{wmi};
    action.set_query(L"SELECT Model FROM Win32_ComputerSystem");
    action.set_property(L"Model");
    action.set_variable(L"OSDModel");
    action.execute(ctx);

    REQUIRE_FALSE(vars.get(L"OSDModel").has_value());
}
