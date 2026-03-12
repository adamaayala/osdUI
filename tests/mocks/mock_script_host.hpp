#pragma once
#include <osdui/interfaces.hpp>
#include <vector>
#include <queue>

namespace osdui::test {

struct CapturedInvocation {
    std::wstring language;
    std::wstring script;
};

// Queue-based: enqueue expected results in order; throws if queue exhausted.
class CapturingScriptHost : public IScriptHost {
public:
    void enqueue(int exit_code, std::wstring output = {}) {
        results_.push({exit_code, std::move(output)});
    }
    model::ScriptResult execute(std::wstring_view language,
                                std::wstring_view script) override {
        invocations_.push_back({std::wstring{language}, std::wstring{script}});
        if (results_.empty()) return {0, {}};  // default: success, no output
        auto r = std::move(results_.front());
        results_.pop();
        return r;
    }
    const std::vector<CapturedInvocation>& invocations() const { return invocations_; }
private:
    std::vector<CapturedInvocation>       invocations_;
    std::queue<model::ScriptResult>       results_;
};

} // namespace osdui::test
