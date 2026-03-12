#pragma once
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include "interfaces.hpp"

namespace osdui {

// ── IAction ──────────────────────────────────────────────────────────────────

struct ActionContext {
    IVariableStore&      vars;
    IDialogPresenter&    dialogs;
    IScriptHost&         scripts;
    IConditionEvaluator& conditions;
};

enum class ActionOutcome { Continue, Abort, JumpTo };

struct ActionResult {
    ActionOutcome outcome{ActionOutcome::Continue};
    std::wstring  jump_target;  // node id when outcome == JumpTo
};

struct IAction {
    virtual ~IAction() = default;
    virtual ActionResult execute(ActionContext& ctx) = 0;
    // Optional condition expression; empty = always run
    std::wstring condition;
};

// ── ActionGraph ──────────────────────────────────────────────────────────────

struct ActionNode {
    std::wstring          id;    // optional XML id= attribute
    std::unique_ptr<IAction> action;
};

struct ActionGraph {
    std::vector<ActionNode> nodes;

    // Returns index of node with given id, or npos if not found
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
    std::size_t find(std::wstring_view id) const noexcept {
        for (std::size_t i = 0; i < nodes.size(); ++i)
            if (nodes[i].id == id) return i;
        return npos;
    }
};

} // namespace osdui
