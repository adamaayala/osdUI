#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/interfaces.hpp>
#include "../logging/cm_log.hpp"

namespace osdui::actions {

enum class RunResult { Success, Aborted };

class ActionRunner {
public:
    // log is optional — pass nullptr to disable logging (useful in tests)
    RunResult run(const ActionGraph&   graph,
                  IVariableStore&      vars,
                  IDialogPresenter&    dialogs,
                  IScriptHost&         scripts,
                  IConditionEvaluator& conditions,
                  logging::CmLog*      log = nullptr);
};

} // namespace osdui::actions
