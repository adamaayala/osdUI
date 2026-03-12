#include "dialog_save_items.hpp"
#include "dialog_data.hpp"
#include "resource.h"
#include <commctrl.h>

namespace osdui::dialogs {

namespace {

void populate_save_list(HWND list, const model::DialogSpec& spec) {
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    col.fmt  = LVCFMT_LEFT;

    col.pszText = const_cast<LPWSTR>(L"Item");
    col.cx      = 200;
    ListView_InsertColumn(list, 0, &col);

    col.pszText = const_cast<LPWSTR>(L"Value");
    col.cx      = 100;
    ListView_InsertColumn(list, 1, &col);

    int row = 0;
    for (const auto& input : spec.inputs) {
        LVITEMW item{};
        item.mask     = LVIF_TEXT;
        item.iItem    = row;
        item.iSubItem = 0;

        std::wstring name_buf = input.label;
        item.pszText = name_buf.data();
        ListView_InsertItem(list, &item);

        std::wstring val_buf = input.default_value;
        ListView_SetItemText(list, row, 1, val_buf.data());

        ++row;
    }
}

INT_PTR CALLBACK save_items_proc(HWND hwnd, UINT msg,
                                 WPARAM wp, LPARAM lp) {
    auto* data = reinterpret_cast<DlgData*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lp);
        data = reinterpret_cast<DlgData*>(lp);

        SetWindowTextW(hwnd, data->spec->title.c_str());

        HWND list = GetDlgItem(hwnd, IDC_SAVE_LIST);
        ListView_SetExtendedListViewStyle(
            list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        populate_save_list(list, *data->spec);

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

model::DialogResult show_save_items(HINSTANCE inst,
                                    const model::DialogSpec& spec) {
    DlgData data{&spec, {}};

    DialogBoxParamW(inst,
                    MAKEINTRESOURCEW(IDD_SAVE_ITEMS),
                    nullptr,
                    save_items_proc,
                    reinterpret_cast<LPARAM>(&data));

    return data.result;
}

} // namespace osdui::dialogs
