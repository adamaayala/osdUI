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
};

// ── Dialog spec (passed to IDialogPresenter) ─────────────────────────────────

struct DialogSpec {
    std::wstring title;
    std::wstring banner_title;
    std::wstring banner_text;
    std::vector<InputSpec> inputs;
    bool allow_cancel{false};
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
    std::wstring category;
    bool         required{false};
};

// ── Action spec (used by config parser to carry raw XML attributes) ───────────

struct ActionSpec {
    std::wstring type;       // XML Type= attribute
    std::wstring id;         // XML id= attribute (optional)
    std::wstring condition;  // XML Condition= attribute (optional)
};

} // namespace osdui::model
