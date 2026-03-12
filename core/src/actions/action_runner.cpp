#include "action_runner.hpp"
#include <format>

namespace osdui::actions {

RunResult ActionRunner::run(const ActionGraph&   graph,
                            IVariableStore&      vars,
                            IDialogPresenter&    dialogs,
                            IScriptHost&         scripts,
                            IConditionEvaluator& conditions,
                            logging::CmLog*      log)
{
    ActionContext ctx{vars, dialogs, scripts, conditions};
    std::size_t i = 0;

    while (i < graph.nodes.size()) {
        const auto& node = graph.nodes[i];

        // Evaluate action-level condition
        if (!node.action->condition.empty()) {
            if (!conditions.evaluate(node.action->condition, vars)) {
                ++i;
                continue;
            }
        }

        ActionResult result = node.action->execute(ctx);

        switch (result.outcome) {
            case ActionOutcome::Continue:
                ++i;
                break;
            case ActionOutcome::Abort:
                return RunResult::Aborted;
            case ActionOutcome::JumpTo: {
                auto target = graph.find(result.jump_target);
                if (target == ActionGraph::npos) {
                    if (log) log->error(L"ActionRunner",
                        std::format(L"JumpTo target '{}' not found — aborting",
                                    result.jump_target));
                    return RunResult::Aborted;
                }
                i = target;
                break;
            }
        }
    }
    return RunResult::Success;
}

} // namespace osdui::actions
