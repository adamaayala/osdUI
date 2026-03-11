# osdUI Modernization Design

**Date:** 2026-03-11
**Status:** Approved
**Project:** osdUI (modernized rewrite of UI++)

---

## Overview

osdUI is a clean C++20 rewrite of the abandoned UI++ project — a user interface harness for Microsoft Configuration Manager (SCCM/ConfigMgr) OS Deployment (OSD) task sequences. It presents dialogs, collects user input, runs preflight checks, executes scripts and REST calls, and writes results back as task sequence variables.

The rewrite drops LDAP (`AD`) and AWS integrations (unused) and `UserAuth`, replaces MFC with Win32+wil, replaces MSBuild with CMake, and introduces a headless core/UI split for testability.

**Backward compatibility:** osdUI must parse existing UI++ XML config files without modification. The XML schema (root `UIpp` → `Actions` → `Action type="..."`) is preserved exactly.

---

## Architecture

Two CMake targets:

- **`osdui-core`** — static library, pure C++20, no Win32 UI dependency
- **`osdui.exe`** — thin Win32+wil executable, wires platform implementations to core

**Linking:** All vcpkg dependencies and the CRT are linked statically (`/MT`). Single deployable `.exe`, no runtime dependencies beyond the OS. WinPE-compatible — only Win32 APIs available in WinPE are used.

---

## Directory Layout

```
osdUI/
├── CMakeLists.txt
├── vcpkg.json                 ← pugixml, libcurl, wil, nlohmann-json, catch2
├── core/                      ← osdui-core static lib
│   ├── include/osdui/         ← public headers (model, interfaces)
│   └── src/
│       ├── config/            ← XML parsing → ActionGraph
│       ├── actions/           ← one file per action type
│       ├── model/             ← value types (Action, Dialog, Input, etc.)
│       ├── http/              ← curl-based REST client
│       ├── script/            ← IScriptHost interface + condition evaluator
│       └── logging/           ← CM log format writer
├── app/                       ← osdui.exe
│   ├── dialogs/               ← Win32 dialog implementations
│   ├── platform/              ← TsVariables (COM), WshScriptHost
│   └── main.cpp
├── docs/                      ← documentation
│   └── superpowers/specs/
└── tests/                     ← Catch2 unit tests (core only)
    └── fixtures/              ← XML fixture files for config parsing tests
```

---

## Data Flow

```
main.cpp  →  ConfigParser  →  ActionGraph  →  ActionRunner
                                                    ↓
                                          IVariableStore  (read/write TS vars)
                                          IDialogPresenter (show dialog, return results)
                                          IScriptHost      (execute script / eval condition)
                                          IConditionEvaluator (evaluate boolean expressions)
```

1. `main.cpp` parses CLI args (config file path, log path)
2. `ConfigParser` reads XML into an `ActionGraph`
3. `ActionRunner` executes each action in sequence; `Switch` actions may redirect flow
4. Actions interact with the environment exclusively through the four interfaces
5. On completion, all TS variable writes have been committed via `IVariableStore`

---

## ActionGraph

`ActionGraph` is an ordered sequence of `ActionNode` values. It is not a DAG — execution is always linear, with `Switch` actions redirecting the runner to a named target action by index. There is no implicit branching at parse time; all conditions are evaluated at runtime.

```cpp
namespace osdui {
  struct ActionNode {
    std::wstring id;           // optional XML id= attribute
    std::unique_ptr<IAction> action;
  };

  struct ActionGraph {
    std::vector<ActionNode> nodes;
    // Returns index of node with given id, or std::numeric_limits<std::size_t>::max() if not found
    std::size_t find(std::wstring_view id) const noexcept;
  };
}
```

`ActionRunner` iterates `nodes` with an integer cursor. A `Switch` result can set the cursor to any valid index. Unknown node ids from `Switch` are a runtime error (logged + abort).

---

## Key Interfaces

### `IVariableStore`
```cpp
namespace osdui {
  struct IVariableStore {
    virtual ~IVariableStore() = default;
    virtual std::optional<std::wstring> get(std::wstring_view name) const = 0;
    // Throws wil::ResultException on COM failure in production impl
    virtual void set(std::wstring_view name, std::wstring_view value) = 0;
  };
}
```
- **Production:** wraps COM `ITSEnvClass`; uses `wil::check_hresult` — COM failures throw `wil::ResultException`, caught by `ActionRunner` and logged as fatal errors
- **Tests:** `MapVariableStore` backed by `std::map<std::wstring, std::wstring>`

### `IDialogPresenter`
```cpp
namespace osdui {
  struct DialogResult {
    bool accepted;                                  // false = user cancelled
    std::map<std::wstring, std::wstring> values;    // empty if !accepted
  };
  struct IDialogPresenter {
    virtual ~IDialogPresenter() = default;
    virtual DialogResult present(const model::DialogSpec& spec,
                                 const IVariableStore& vars) = 0;
  };
}
```
- **Cancel contract:** when `accepted == false`, `values` is always empty. `ActionRunner` does not write any variables from a cancelled dialog; it logs the cancellation and exits with a non-zero code.
- **Production:** Win32 dialog driven by the spec
- **Tests:** `ScriptedDialogPresenter` returns canned responses

### `IScriptHost`
```cpp
namespace osdui {
  struct ScriptResult {
    int exit_code;
    std::wstring output;   // stdout capture where applicable
  };
  struct IScriptHost {
    virtual ~IScriptHost() = default;
    virtual ScriptResult execute(std::wstring_view language,
                                 std::wstring_view script) = 0;
  };
}
```
- Used by `ExternalCall` (raw process execution via `CreateProcess`) and script-language actions (VBScript, JScript via `IActiveScriptParse`). `IScriptHost` covers both — the production implementation dispatches on `language`: empty/`"exe"` → `CreateProcess`; otherwise → WSH.
- **Production:** Windows Script Host via `IActiveScriptParse`
- **Tests:** `CapturingScriptHost` records invocations for assertion

### `IConditionEvaluator`
```cpp
namespace osdui {
  struct IConditionEvaluator {
    virtual ~IConditionEvaluator() = default;
    virtual bool evaluate(std::wstring_view expression,
                          const IVariableStore& vars) const = 0;
  };
}
```
- Evaluates boolean expressions on XML `condition=`, `checkcondition=`, `warncondition=` attributes
- Used by `ActionRunner` before executing any action, and by individual actions for per-control conditions
- **Production:** expression parser (variable substitution + comparison operators); falls back to WSH for complex script expressions
- **Tests:** `LiteralConditionEvaluator` accepts a map of expression→bool for deterministic test control

---

## In-Scope Action Types

All action types from the original except `AD` (LDAP), AWS, and `UserAuth`.

| XML type | Description |
|---|---|
| `Input` | User input dialog — text fields, dropdowns, checkboxes |
| `Info` | User info/message dialog |
| `InfoFullScreen` | Full-screen variant of Info |
| `AppTree` | Software/application selection tree |
| `DefaultValues` | Set TS variable defaults without showing UI |
| `ExternalCall` | Run an executable; capture exit code as TS var |
| `Preflight` | Evaluate conditions and display pass/fail/warn results |
| `RegRead` | Read registry value into TS var |
| `RegWrite` | Write TS var value to registry |
| `WMIRead` | WMI query result into TS var |
| `WMIWrite` | Write value via WMI |
| `FileRead` | Read file content into TS var |
| `Switch` | Conditional branch — redirect action cursor by id |
| `TSVar` | Display current TS variable values (debug dialog) |
| `TSVarList` | TS variable list operations |
| `RandomString` | Generate random string into TS var |
| `SoftwareDiscovery` | Read installed software from registry |
| `SaveItems` | Export selected items to a file |
| `Match` | Pattern match input against rules |
| `ErrorInfo` | Error information dialog |
| `Vars` | Variable viewer dialog |
| REST | HTTP REST call; response into TS var (was commented out in original — reimplemented) |

## Out of Scope

| XML type | Reason |
|---|---|
| `AD` | LDAP / Active Directory — not used |
| `TPM` | TPM chip status checks — not used |
| `UserAuth` | User authentication dialog — not used |
| AWS extension | Not used |

---

## Error Handling

- XML parse errors → CM log entry + exit non-zero (task sequence marks step failed)
- Unknown action type in XML → CM log warning + action skipped (permissive for forward compat)
- Missing required TS vars → CM log entry + configurable per-action: abort or continue
- Cancelled dialog → CM log entry + exit non-zero
- `IVariableStore::set` COM failure → `wil::ResultException` thrown → caught by `ActionRunner` → CM log entry + exit non-zero
- HTTP errors → CM log entry + action result carries error; downstream `Switch` can branch on it
- Script/ExternalCall errors → exit code captured as TS var; CM log entry
- Condition evaluation errors → treated as `false`; CM log warning
- No silent failures — every error path produces at least one CM log entry

---

## Dependencies (vcpkg)

| Package | Purpose | Notes |
|---|---|---|
| `pugixml` | XML parsing | Fast, header-friendly |
| `libcurl` | HTTP client | Pinned at 7.83.1 to match the original known-working build. Known CVEs are accepted technical debt in a closed OSD network environment. **Unpinning and validating against the latest release is a prerequisite before any production deployment.** |
| `wil` | Win32 RAII wrappers | Smart handles, `check_hresult`, error propagation |
| `nlohmann-json` | REST response parsing | |
| `catch2` | Unit testing | `tests/` target only |

---

## Testing Strategy

- **Catch2** unit tests in `tests/` target `osdui-core` only — no Win32 required to build or run on a dev machine
- Each action tested with `MapVariableStore` + `ScriptedDialogPresenter` + `CapturingScriptHost` + `LiteralConditionEvaluator`
- XML parsing tested against fixture `.xml` files in `tests/fixtures/`
- HTTP client tested against recorded response fixtures
- UI layer (`app/`) tested manually during OSD lab runs; Win32 dialog correctness (cancel handling, variable commit) verified via checklist before each release

---

## Build

```bash
# Configure
cmake --preset windows-release   # x64 Release, static CRT (/MT)
cmake --preset windows-debug      # x64 Debug

# Build
cmake --build --preset windows-release

# Test (core only — no Win32 environment required)
ctest --preset windows-release
```

Both `Win32` and `x64` targets. CI via GitHub Actions on `windows-latest`, running build + `ctest` for both platforms on every push.
