#include "dialog_preflight.hpp"
#include "dialog_data.hpp"
#include "resource.h"
#include <commctrl.h>

namespace osdui::dialogs {

namespace {

void populate_preflight_list(HWND list, const model::DialogSpec& spec) {
    // Set up columns: Name and Status.
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    col.fmt  = LVCFMT_LEFT;

    col.pszText = const_cast<LPWSTR>(L"Check");
    col.cx      = 200;
    ListView_InsertColumn(list, 0, &col);

    col.pszText = const_cast<LPWSTR>(L"Status");
    col.cx      = 80;
    ListView_InsertColumn(list, 1, &col);

    int row = 0;
    for (const auto& input : spec.inputs) {
        LVITEMW item{};
        item.mask     = LVIF_TEXT;
        item.iItem    = row;
        item.iSubItem = 0;

        // Use a local copy for the non-const pszText pointer.
        std::wstring name_buf = input.label;
        item.pszText = name_buf.data();
        ListView_InsertItem(list, &item);

        // Derive a status string from the default_value field.
        // The action engine is expected to set default_value to
        // "Pass", "Warn", or "Fail" before presenting.
        std::wstring status_buf = input.default_value;
        ListView_SetItemText(list, row, 1, status_buf.data());

        ++row;
    }
}

INT_PTR CALLBACK preflight_proc(HWND hwnd, UINT msg,
                                WPARAM wp, LPARAM lp) {
    auto* data = reinterpret_cast<DlgData*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lp);
        data = reinterpret_cast<DlgData*>(lp);

        SetWindowTextW(hwnd, data->spec->title.c_str());

        HWND list = GetDlgItem(hwnd, IDC_PREFLIGHT_LIST);
        ListView_SetExtendedListViewStyle(
            list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        populate_preflight_list(list, *data->spec);

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

model::DialogResult show_preflight(HINSTANCE inst,
                                   const model::DialogSpec& spec) {
    DlgData data{&spec, {}};

    DialogBoxParamW(inst,
                    MAKEINTRESOURCEW(IDD_PREFLIGHT),
                    nullptr,
                    preflight_proc,
                    reinterpret_cast<LPARAM>(&data));

    return data.result;
}

} // namespace osdui::dialogs
