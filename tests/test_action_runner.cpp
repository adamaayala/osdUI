#include <catch2/catch_test_macros.hpp>
#include "actions/action_runner.hpp"
#include "script/condition_evaluator.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"
#include <osdui/action_graph.hpp>

using namespace osdui;
using namespace osdui::test;

// A simple action that records its execution
struct RecordingAction : IAction {
    bool* executed;
    explicit RecordingAction(bool& flag) : executed{&flag} {}
    ActionResult execute(ActionContext&) override { *executed = true; return {}; }
};

TEST_CASE("ActionRunner executes all actions") {
    ActionGraph graph;
    bool a_ran = false, b_ran = false;
    graph.nodes.push_back({L"a", std::make_unique<RecordingAction>(a_ran)});
    graph.nodes.push_back({L"b", std::make_unique<RecordingAction>(b_ran)});

    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;

    actions::ActionRunner runner;
    auto result = runner.run(graph, vars, dlg, sh, ce);

    REQUIRE(result == actions::RunResult::Success);
    REQUIRE(a_ran);
    REQUIRE(b_ran);
}

TEST_CASE("ActionRunner skips action with false condition") {
    ActionGraph graph;
    bool ran = false;
    auto action = std::make_unique<RecordingAction>(ran);
    action->condition = L"ShouldNotRun";
    graph.nodes.push_back({L"", std::move(action)});

    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    ce.set(L"ShouldNotRun", false);

    actions::ActionRunner runner;
    runner.run(graph, vars, dlg, sh, ce);
    REQUIRE_FALSE(ran);
}

TEST_CASE("ActionRunner handles JumpTo - skips intervening nodes") {
    ActionGraph graph;
    bool skipped_ran = false;
    bool target_ran  = false;

    struct JumpAction : IAction {
        ActionResult execute(ActionContext&) override {
            return {ActionOutcome::JumpTo, L"target"};
        }
    };
    struct TargetAction : IAction {
        bool* ran;
        explicit TargetAction(bool& f) : ran{&f} {}
        ActionResult execute(ActionContext&) override { *ran = true; return {}; }
    };

    graph.nodes.push_back({L"jumper",  std::make_unique<JumpAction>()});
    graph.nodes.push_back({L"skipped", std::make_unique<RecordingAction>(skipped_ran)});
    graph.nodes.push_back({L"target",  std::make_unique<TargetAction>(target_ran)});

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    actions::ActionRunner runner;
    runner.run(graph, vars, dlg, sh, ce);

    REQUIRE_FALSE(skipped_ran);
    REQUIRE(target_ran);
}
