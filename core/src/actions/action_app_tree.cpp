#include "action_app_tree.hpp"

namespace osdui::actions {

ActionResult AppTreeAction::execute(ActionContext& ctx) {
    model::DialogSpec spec{ .type = model::DialogType::AppTree, .title = title_ };
    // Present the software list as info items — Win32 dialog shows tree
    for (const auto& item : items_) {
        spec.inputs.push_back(model::InputSpec{
            .variable      = item.id,
            .label         = item.name,
            .type          = model::InputType::Info,
            .default_value = item.required ? L"true" : L"",
        });
    }
    auto result = ctx.dialogs.present(spec, ctx.vars);
    // Write selected items to vars based on DialogResult::values
    for (const auto& [key, value] : result.values)
        ctx.vars.set(key, value);
    return {};
}

} // namespace osdui::actions
