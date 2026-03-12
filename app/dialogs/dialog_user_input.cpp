#include "dialog_user_input.hpp"
#include "dialog_data.hpp"
#include "resource.h"
#include <vector>

namespace osdui::dialogs {

namespace {

// Tracks a dynamically created control and its associated variable name.
struct FieldControl {
    std::wstring variable;
    HWND         hwnd{nullptr};
    model::InputType type{model::InputType::Text};
};

// Base ID for dynamically created controls.
constexpr int DYNAMIC_CONTROL_BASE = 3000;

// Layout constants (dialog units approximated as pixels).
constexpr int LABEL_HEIGHT   = 16;
constexpr int CONTROL_HEIGHT = 22;
constexpr int FIELD_GAP      = 6;
constexpr int LEFT_MARGIN    = 7;
constexpr int RIGHT_MARGIN   = 7;

struct InputDlgData {
    DlgData                  base;
    std::vector<FieldControl> fields;
};

void create_dynamic_controls(HWND hwnd, InputDlgData* data) {
    RECT client{};
    GetClientRect(hwnd, &client);
    const int field_width = client.right - LEFT_MARGIN - RIGHT_MARGIN;

    int y = 30;
    int control_id = DYNAMIC_CONTROL_BASE;

    for (const auto& input : data->base.spec->inputs) {
        // Label
        CreateWindowExW(0, L"STATIC", input.label.c_str(),
                        WS_CHILD | WS_VISIBLE | SS_LEFT,
                        LEFT_MARGIN, y, field_width, LABEL_HEIGHT,
                        hwnd, reinterpret_cast<HMENU>(
                            static_cast<INT_PTR>(control_id++)),
                        nullptr, nullptr);
        y += LABEL_HEIGHT + 2;

        FieldControl fc{input.variable, nullptr, input.type};

        switch (input.type) {
        case model::InputType::Text:
        case model::InputType::Password: {
            DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
            if (input.type == model::InputType::Password) {
                style |= ES_PASSWORD;
            }
            fc.hwnd = CreateWindowExW(
                0, L"EDIT", input.default_value.c_str(), style,
                LEFT_MARGIN, y, field_width, CONTROL_HEIGHT,
                hwnd, reinterpret_cast<HMENU>(
                    static_cast<INT_PTR>(control_id++)),
                nullptr, nullptr);
            break;
        }
        case model::InputType::Dropdown: {
            fc.hwnd = CreateWindowExW(
                0, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                LEFT_MARGIN, y, field_width, CONTROL_HEIGHT * 6,
                hwnd, reinterpret_cast<HMENU>(
                    static_cast<INT_PTR>(control_id++)),
                nullptr, nullptr);

            int selected_index = 0;
            int idx = 0;
            for (const auto& item : input.items) {
                SendMessageW(fc.hwnd, CB_ADDSTRING, 0,
                             reinterpret_cast<LPARAM>(item.display.c_str()));
                if (item.value == input.default_value) {
                    selected_index = idx;
                }
                ++idx;
            }
            SendMessageW(fc.hwnd, CB_SETCURSEL, selected_index, 0);
            break;
        }
        case model::InputType::Checkbox: {
            fc.hwnd = CreateWindowExW(
                0, L"BUTTON", input.label.c_str(),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                LEFT_MARGIN, y, field_width, CONTROL_HEIGHT,
                hwnd, reinterpret_cast<HMENU>(
                    static_cast<INT_PTR>(control_id++)),
                nullptr, nullptr);

            if (input.default_value == L"true" ||
                input.default_value == L"1") {
                SendMessageW(fc.hwnd, BM_SETCHECK, BST_CHECKED, 0);
            }
            break;
        }
        case model::InputType::Info: {
            // Info fields are display-only; no harvesting needed.
            CreateWindowExW(0, L"STATIC", input.label.c_str(),
                            WS_CHILD | WS_VISIBLE | SS_LEFT,
                            LEFT_MARGIN, y, field_width, CONTROL_HEIGHT,
                            hwnd, reinterpret_cast<HMENU>(
                                static_cast<INT_PTR>(control_id++)),
                            nullptr, nullptr);
            fc.hwnd = nullptr; // nothing to harvest
            break;
        }
        }

        y += CONTROL_HEIGHT + FIELD_GAP;
        data->fields.push_back(fc);
    }
}

void harvest_values(InputDlgData* data) {
    const auto& inputs = data->base.spec->inputs;

    for (size_t i = 0; i < data->fields.size(); ++i) {
        const auto& fc = data->fields[i];
        if (fc.variable.empty() || fc.hwnd == nullptr) {
            continue;
        }

        switch (fc.type) {
        case model::InputType::Text:
        case model::InputType::Password: {
            int len = GetWindowTextLengthW(fc.hwnd);
            std::wstring buf(static_cast<size_t>(len) + 1, L'\0');
            GetWindowTextW(fc.hwnd, buf.data(), len + 1);
            buf.resize(static_cast<size_t>(len));
            data->base.result.values[fc.variable] = buf;
            break;
        }
        case model::InputType::Dropdown: {
            auto sel = static_cast<int>(
                SendMessageW(fc.hwnd, CB_GETCURSEL, 0, 0));
            if (sel >= 0 && i < inputs.size() &&
                static_cast<size_t>(sel) < inputs[i].items.size()) {
                data->base.result.values[fc.variable] =
                    inputs[i].items[static_cast<size_t>(sel)].value;
            }
            break;
        }
        case model::InputType::Checkbox: {
            auto checked = SendMessageW(fc.hwnd, BM_GETCHECK, 0, 0);
            data->base.result.values[fc.variable] =
                (checked == BST_CHECKED) ? L"true" : L"false";
            break;
        }
        case model::InputType::Info:
            break;
        }
    }
}

INT_PTR CALLBACK user_input_proc(HWND hwnd, UINT msg,
                                 WPARAM wp, LPARAM lp) {
    auto* data = reinterpret_cast<InputDlgData*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lp);
        data = reinterpret_cast<InputDlgData*>(lp);

        SetWindowTextW(hwnd, data->base.spec->title.c_str());

        if (!data->base.spec->allow_cancel) {
            EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
            ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
        }

        create_dynamic_controls(hwnd, data);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == IDOK) {
            harvest_values(data);
            data->base.result.accepted = true;
            EndDialog(hwnd, IDOK);
            return TRUE;
        }
        if (LOWORD(wp) == IDCANCEL) {
            data->base.result.accepted = false;
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

} // namespace

model::DialogResult show_user_input(HINSTANCE inst,
                                    const model::DialogSpec& spec) {
    InputDlgData data{{&spec, {}}, {}};

    DialogBoxParamW(inst,
                    MAKEINTRESOURCEW(IDD_USER_INPUT),
                    nullptr,
                    user_input_proc,
                    reinterpret_cast<LPARAM>(&data));

    return data.base.result;
}

} // namespace osdui::dialogs
