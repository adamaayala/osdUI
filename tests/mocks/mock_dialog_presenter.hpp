#pragma once
#include <osdui/interfaces.hpp>
#include <queue>
#include <stdexcept>

namespace osdui::test {

// Returns pre-queued results in order; throws if queue is exhausted.
class ScriptedDialogPresenter : public IDialogPresenter {
public:
    void enqueue(model::DialogResult result) {
        queue_.push(std::move(result));
    }
    model::DialogResult present(const model::DialogSpec&,
                                const IVariableStore&) override {
        if (queue_.empty()) throw std::logic_error{"ScriptedDialogPresenter: no more results"};
        auto r = std::move(queue_.front());
        queue_.pop();
        return r;
    }
private:
    std::queue<model::DialogResult> queue_;
};

} // namespace osdui::test
