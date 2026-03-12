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
    const IVariableStore& /*vars  — available for variable substitution in dialog text
        (e.g., resolving %VAR% in labels).  Currently not forwarded to dialog
        implementations — deferred until field-level variable substitution is
        needed.  See TODO(chunk7). */) {

    switch (spec.type) {
    case model::DialogType::UserInput:     return show_user_input(instance_, spec);
    case model::DialogType::UserInfo:      return show_user_info(instance_, spec);
    case model::DialogType::InfoFullScreen:return show_user_info(instance_, spec);  // reuse
    case model::DialogType::Preflight:     return show_preflight(instance_, spec);
    case model::DialogType::AppTree:       return show_app_tree(instance_, spec);
    case model::DialogType::TsVar:         return show_ts_var(instance_, spec);
    case model::DialogType::Vars:          return show_vars(instance_, spec);
    case model::DialogType::ErrorInfo:     return show_error_info(instance_, spec);
    case model::DialogType::SaveItems:     return show_save_items(instance_, spec);
    }
    return {};  // unreachable — all enum values handled above
}

} // namespace osdui::dialogs
