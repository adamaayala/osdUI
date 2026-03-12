#include "dialog_app_tree.hpp"
#include "dialog_data.hpp"
#include "resource.h"
#include <commctrl.h>

namespace osdui::dialogs {

namespace {

void populate_tree(HWND tree, const model::DialogSpec& spec) {
    for (const auto& input : spec.inputs) {
        TVINSERTSTRUCTW tvi{};
        tvi.hParent      = TVI_ROOT;
        tvi.hInsertAfter = TVI_LAST;
        tvi.item.mask    = TVIF_TEXT | TVIF_PARAM;

        std::wstring label_buf = input.label;
        tvi.item.pszText = label_buf.data();
        tvi.item.lParam  = 0;

        TreeView_InsertItem(tree, &tvi);
    }
}

void harvest_checked_items(HWND tree, DlgData* data) {
    const auto& inputs = data->spec->inputs;
    HTREEITEM item = TreeView_GetRoot(tree);
    size_t idx = 0;

    while (item != nullptr && idx < inputs.size()) {
        UINT state = TreeView_GetCheckState(tree, item);
        if (state != 0) {
            data->result.values[inputs[idx].variable] = L"true";
        }
        item = TreeView_GetNextSibling(tree, item);
        ++idx;
    }
}

INT_PTR CALLBACK app_tree_proc(HWND hwnd, UINT msg,
                               WPARAM wp, LPARAM lp) {
    auto* data = reinterpret_cast<DlgData*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lp);
        data = reinterpret_cast<DlgData*>(lp);

        SetWindowTextW(hwnd, data->spec->title.c_str());

        HWND tree = GetDlgItem(hwnd, IDC_APP_TREE_LIST);
        populate_tree(tree, *data->spec);

        if (!data->spec->allow_cancel) {
            EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
            ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
        }

        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == IDOK) {
            HWND tree = GetDlgItem(hwnd, IDC_APP_TREE_LIST);
            harvest_checked_items(tree, data);
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

model::DialogResult show_app_tree(HINSTANCE inst,
                                  const model::DialogSpec& spec) {
    DlgData data{&spec, {}};

    DialogBoxParamW(inst,
                    MAKEINTRESOURCEW(IDD_APP_TREE),
                    nullptr,
                    app_tree_proc,
                    reinterpret_cast<LPARAM>(&data));

    return data.result;
}

} // namespace osdui::dialogs
