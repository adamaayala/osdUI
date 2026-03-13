#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

namespace osdui::model {

// ── Results ───────────────────────────────────────────────────────────────────

struct ScriptResult {
    int         exit_code{0};
    std::wstring output;
};

struct DialogResult {
    bool accepted{false};
    // Always empty when accepted == false
    std::map<std::wstring, std::wstring> values;
};

// ── Input controls (used by DialogSpec) ──────────────────────────────────────

enum class InputType { Text, Password, Dropdown, Checkbox, Info };

enum class DialogType {
    UserInput,
    UserInfo,
    InfoFullScreen,
    Preflight,
    AppTree,
    TsVar,
    Vars,
    ErrorInfo,
    SaveItems,
};

struct DropdownItem {
    std::wstring value;
    std::wstring display;
};

struct InputSpec {
    std::wstring  variable;       // TS variable name
    std::wstring  label;
    InputType     type{InputType::Text};
    std::wstring  default_value;
    bool          required{false};
    std::wstring  condition;      // optional: shown only when condition is true
    std::vector<DropdownItem> items;  // for Dropdown

    // Extended attributes from real UI++ XML
    std::wstring  hint;            // Hint= watermark/tooltip text
    std::wstring  regex;           // RegEx= validation pattern
    std::wstring  force_case;      // ForceCase= Upper/Lower/No
    std::wstring  checked_value;   // CheckedValue= for Checkbox
    std::wstring  unchecked_value; // UncheckedValue= for Checkbox
};

// ── Dialog spec (passed to IDialogPresenter) ─────────────────────────────────

struct DialogSpec {
    DialogType type{DialogType::UserInput};
    std::wstring title;
    std::wstring banner_title;
    std::wstring banner_text;
    std::vector<InputSpec> inputs;
    bool allow_cancel{false};
    std::vector<SoftwareGroup> groups;  // AppTree only
};

// ── Preflight check ──────────────────────────────────────────────────────────

enum class PreflightStatus { Pass, Warn, Fail };

struct PreflightItem {
    std::wstring    name;
    std::wstring    condition;
    std::wstring    warn_condition;
    PreflightStatus status{PreflightStatus::Fail};  // set at runtime
};

// ── Software item (AppTree) ──────────────────────────────────────────────────

struct SoftwareItem {
    std::wstring id;
    std::wstring name;
    std::wstring label;           // display label (from catalog Application Label=)
    std::wstring category;
    bool         required{false};
    bool         default_selected{false};
    bool         hidden{false};
};

struct SoftwareGroup {
    std::wstring id;
    std::wstring label;
    bool         default_expanded{false};
    bool         required{false};
    std::vector<SoftwareItem> items;
};

// ── Action spec (used by config parser to carry raw XML attributes) ───────────

struct ActionSpec {
    std::wstring type;       // XML Type= attribute
    std::wstring id;         // XML id= attribute (optional)
    std::wstring condition;  // XML Condition= attribute (optional)
};

} // namespace osdui::model
