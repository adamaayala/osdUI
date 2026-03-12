#include "dialog_presenter.hpp"
#include "dialog_user_info.hpp"
#include "dialog_user_input.hpp"
#include "dialog_preflight.hpp"
#include "dialog_app_tree.hpp"
#include "dialog_ts_var.hpp"
#include "dialog_vars.hpp"
#include "dialog_error_info.hpp"
#include "dialog_save_items.hpp"

namespace osdui::dialogs {

DialogPresenter::DialogPresenter(HINSTANCE instance)
    : instance_{instance} {}

model::DialogResult DialogPresenter::present(
    const model::DialogSpec& spec,
    const IVariableStore& /*vars*/) {

    // Dispatch based on the composition of input types in the spec.
    // The action engine populates DialogSpec differently for each dialog
    // kind; we inspect the inputs to determine which dialog to show.

    if (spec.inputs.empty()) {
        // No inputs at all — treat as informational.
        return show_user_info(instance_, spec);
    }

    // Check if all inputs share a single type that maps to a specific dialog.
    const auto first_type = spec.inputs[0].type;
    bool all_same_type = true;
    for (const auto& input : spec.inputs) {
        if (input.type != first_type) {
            all_same_type = false;
            break;
        }
    }

    // Pure informational display (all Info inputs).
    if (all_same_type && first_type == model::InputType::Info) {
        return show_user_info(instance_, spec);
    }

    // Mixed input types or specific interactive types — use the general
    // UserInput dialog which dynamically creates controls per input.
    return show_user_input(instance_, spec);
}

} // namespace osdui::dialogs
