#pragma once
#include <osdui/model.hpp>

namespace osdui::dialogs {

// Shared data structure passed to dialog procedures via LPARAM/GWLP_USERDATA.
struct DlgData {
    const model::DialogSpec* spec;
    model::DialogResult      result;
};

} // namespace osdui::dialogs
