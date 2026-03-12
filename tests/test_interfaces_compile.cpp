#include <catch2/catch_test_macros.hpp>
#include <osdui/interfaces.hpp>
#include <osdui/model.hpp>
#include <osdui/action_graph.hpp>
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"

// If this file compiles, the public headers are well-formed.
TEST_CASE("headers compile") {
    SUCCEED("headers compiled successfully");
}

TEST_CASE("mocks compile") {
    osdui::test::MapVariableStore vars;
    osdui::test::ScriptedDialogPresenter dlg;
    osdui::test::CapturingScriptHost sh;
    osdui::test::LiteralConditionEvaluator ce;
    SUCCEED();
}
