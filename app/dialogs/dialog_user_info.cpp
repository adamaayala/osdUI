#include "dialog_user_info.hpp"
#include "dialog_data.hpp"
#include "resource.h"

namespace osdui::dialogs {

namespace {

INT_PTR CALLBACK user_info_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* data = reinterpret_cast<DlgData*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lp);
        data = reinterpret_cast<DlgData*>(lp);

        SetWindowTextW(hwnd, data->spec->title.c_str());

        if (!data->spec->inputs.empty()) {
            SetDlgItemTextW(hwnd, IDC_INFO_TEXT,
                            data->spec->inputs[0].label.c_str());
        }

        if (!data->spec->allow_cancel) {
            EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
            ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
        }

        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == IDOK) {
            data->result.accepted = true;
            EndDialog(hwnd, IDOK);
            return TRUE;
        }
        if (LOWORD(wp) == IDCANCEL) {
            data->result.accepted = false;
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

} // namespace

model::DialogResult show_user_info(HINSTANCE inst,
                                   const model::DialogSpec& spec) {
    DlgData data{&spec, {}};

    DialogBoxParamW(inst,
                    MAKEINTRESOURCEW(IDD_USER_INFO),
                    nullptr,
                    user_info_proc,
                    reinterpret_cast<LPARAM>(&data));

    return data.result;
}

} // namespace osdui::dialogs
