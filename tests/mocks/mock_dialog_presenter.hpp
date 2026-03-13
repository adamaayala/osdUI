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
    model::DialogResult present(const model::DialogSpec& spec,
                                const IVariableStore&) override {
        last_spec_ = spec;   // store before potential throw
        if (queue_.empty()) throw std::logic_error{"ScriptedDialogPresenter: no more results"};
        auto r = std::move(queue_.front());
        queue_.pop();
        ++calls_made_;
        return r;
    }
    // Number of times present() has been called successfully.
    int calls_made() const { return calls_made_; }
    const model::DialogSpec& last_spec() const { return last_spec_; }
private:
    std::queue<model::DialogResult> queue_;
    int calls_made_{0};
    model::DialogSpec last_spec_;
};

} // namespace osdui::test
