#include <catch2/catch_test_macros.hpp>
#include <osdui/action_graph.hpp>
#include <filesystem>
// ConfigParser not yet included — this will fail to compile first:
#include "../../core/src/config/config_parser.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"

// FIXTURES_DIR is defined by CMake as the absolute path to tests/fixtures/
static const std::filesystem::path fixtures{FIXTURES_DIR};

TEST_CASE("ConfigParser parses basic XML") {
    osdui::config::ConfigParser parser;
    auto graph = parser.parse(fixtures / "basic.xml");
    REQUIRE(graph.nodes.size() == 2);
}

TEST_CASE("ConfigParser: missing file throws") {
    osdui::config::ConfigParser parser;
    REQUIRE_THROWS(parser.parse(fixtures / "nonexistent.xml"));
}

TEST_CASE("ConfigParser: TSVar setter parsed - sets variable, no dialog") {
    osdui::config::ConfigParser parser;
    auto graph = parser.parse(fixtures / "tsvar_set.xml");
    REQUIRE(graph.nodes.size() == 2);

    using namespace osdui;
    using namespace osdui::actions;
    using namespace osdui::test;
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    dlg.enqueue({});  // for the second TSVar (viewer)
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    ActionContext ctx{vars, dlg, sh, ce};

    graph.nodes[0].action->execute(ctx);
    REQUIRE(vars.get(L"BuildStep") == L"5");
    REQUIRE(dlg.calls_made() == 0);  // setter must not show dialog

    graph.nodes[1].action->execute(ctx);
    REQUIRE(dlg.calls_made() == 1);  // viewer does show dialog
}
