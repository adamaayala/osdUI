#include "action_app_tree.hpp"

namespace osdui::actions {

ActionResult AppTreeAction::execute(ActionContext& ctx) {
    model::DialogSpec spec;
    spec.title = title_;
    // Present the software list as info items — Win32 dialog shows tree
    for (const auto& item : items_) {
        model::InputSpec entry;
        entry.type     = model::InputType::Info;
        entry.label    = item.name;
        entry.variable = item.id;
        spec.inputs.push_back(std::move(entry));
    }
    auto result = ctx.dialogs.present(spec, ctx.vars);
    // Write selected items to vars based on DialogResult::values
    for (const auto& [key, value] : result.values)
        ctx.vars.set(key, value);
    return {};
}

} // namespace osdui::actions
