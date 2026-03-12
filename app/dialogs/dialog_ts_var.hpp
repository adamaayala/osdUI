#pragma once
#include <osdui/model.hpp>
#include <windows.h>

namespace osdui::dialogs {

model::DialogResult show_ts_var(HINSTANCE inst,
                                const model::DialogSpec& spec);

} // namespace osdui::dialogs
