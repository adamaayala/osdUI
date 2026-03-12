#include "action_ts_var_list.hpp"
#include <format>

namespace osdui::actions {

ActionResult TSVarListAction::execute(ActionContext& ctx) {
    if (operation_ == L"Append") {
        auto count_val = ctx.vars.get(base_);
        int count = count_val ? std::stoi(*count_val) : 0;
        ++count;
        std::wstring index = std::format(L"{:03d}", count);
        ctx.vars.set(base_ + index, value_);
        ctx.vars.set(base_, std::to_wstring(count));
    } else if (operation_ == L"Count") {
        auto count_val = ctx.vars.get(base_);
        int count = count_val ? std::stoi(*count_val) : 0;
        if (!variable_.empty())
            ctx.vars.set(variable_, std::to_wstring(count));
    } else if (operation_ == L"Remove") {
        auto count_val = ctx.vars.get(base_);
        int count = count_val ? std::stoi(*count_val) : 0;
        // Find and remove matching entry, shift down
        for (int i = 1; i <= count; ++i) {
            std::wstring key = base_ + std::format(L"{:03d}", i);
            auto v = ctx.vars.get(key);
            if (v && *v == value_) {
                // Shift entries down
                for (int j = i; j < count; ++j) {
                    std::wstring from = base_ + std::format(L"{:03d}", j + 1);
                    std::wstring to   = base_ + std::format(L"{:03d}", j);
                    auto next = ctx.vars.get(from);
                    ctx.vars.set(to, next ? *next : L"");
                }
                ctx.vars.set(base_, std::to_wstring(count - 1));
                break;
            }
        }
    }
    return {};
}

} // namespace osdui::actions
