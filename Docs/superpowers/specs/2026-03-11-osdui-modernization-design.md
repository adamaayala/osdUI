# osdUI Modernization Design

**Date:** 2026-03-11
**Status:** Approved
**Project:** osdUI (modernized rewrite of UI++)

---

## Overview

osdUI is a clean C++20 rewrite of the abandoned UI++ project — a user interface harness for Microsoft Configuration Manager (SCCM/ConfigMgr) OS Deployment (OSD) task sequences. It presents dialogs, collects user input, runs preflight checks, executes scripts and REST calls, and writes results back as task sequence variables.

The rewrite drops LDAP and AWS integrations (unused), replaces MFC with Win32+wil, replaces MSBuild with CMake, and introduces a headless core/UI split for testability.

---

## Architecture

Two CMake targets:

- **`osdui-core`** — static library, pure C++20, no Win32 UI dependency
- **`osdui.exe`** — thin Win32+wil executable, wires platform implementations to core

Single deployable `.exe` with no runtime dependencies beyond the OS. WinPE-compatible.

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
│       ├── actions/           ← UserInput, DefaultValues, ExternalCall,
│       │                         Preflight, Software, REST
│       ├── model/             ← value types (Action, Dialog, Input, etc.)
│       ├── http/              ← curl-based REST client
│       ├── script/            ← IScriptHost interface + invoker
│       └── logging/           ← CM log format writer
├── app/                       ← osdui.exe
│   ├── dialogs/               ← Win32 dialog implementations
│   ├── platform/              ← TsVariables (COM), WshScriptHost
│   └── main.cpp
└── tests/                     ← Catch2 unit tests (core only)
```

---

## Data Flow

```
main.cpp  →  ConfigParser  →  ActionGraph  →  ActionRunner
                                                    ↓
                                          IVariableStore (read/write TS vars)
                                          IDialogPresenter (show dialog, return results)
                                          IScriptHost (execute script)
```

1. `main.cpp` parses CLI args (config file path, log path)
2. `ConfigParser` reads XML into an `ActionGraph` (ordered list of actions)
3. `ActionRunner` executes each action in sequence
4. Actions interact with the environment exclusively through the three interfaces
5. On completion, all TS variable writes have been committed via `IVariableStore`

---

## Key Interfaces

These are the testability boundary between core and platform.

### `IVariableStore`
```cpp
namespace osdui {
  struct IVariableStore {
    virtual ~IVariableStore() = default;
    virtual std::optional<std::wstring> get(std::wstring_view name) const = 0;
    virtual void set(std::wstring_view name, std::wstring_view value) = 0;
  };
}
```
- **Production:** wraps COM `ITSEnvClass` (SCCM task sequence environment)
- **Tests:** `MapVariableStore` backed by `std::map<std::wstring, std::wstring>`

### `IDialogPresenter`
```cpp
namespace osdui {
  struct DialogResult {
    bool accepted;
    std::map<std::wstring, std::wstring> values;
  };
  struct IDialogPresenter {
    virtual ~IDialogPresenter() = default;
    virtual DialogResult present(const model::DialogSpec& spec,
                                 const IVariableStore& vars) = 0;
  };
}
```
- **Production:** Win32 dialog driven by the spec
- **Tests:** `ScriptedDialogPresenter` returns canned responses

### `IScriptHost`
```cpp
namespace osdui {
  struct IScriptHost {
    virtual ~IScriptHost() = default;
    virtual int execute(std::wstring_view language,
                        std::wstring_view script) = 0;
  };
}
```
- **Production:** Windows Script Host via COM
- **Tests:** `CapturingScriptHost` records invocations for assertion

---

## In-Scope Features

| Feature | Core component | Notes |
|---|---|---|
| XML config parsing | `config/` | pugixml |
| Task sequence variable R/W | `IVariableStore` | COM in production |
| UserInput dialogs | `actions/`, `app/dialogs/` | Text, checkbox, dropdown, etc. |
| DefaultValues action | `actions/` | Sets vars without UI |
| ExternalCall action | `actions/` | Runs exe, captures exit code |
| Preflight checks | `actions/` | Evaluates conditions, shows results |
| Software selection tree | `actions/`, `app/dialogs/` | App catalog picker |
| REST calls | `http/` | curl + nlohmann-json |
| Script execution | `IScriptHost` | WSH in production |
| CM log format | `logging/` | `.log` files readable by CMTrace |

## Out of Scope

- LDAP / Active Directory queries (removed)
- AWS integration (removed)

---

## Error Handling

- XML parse errors → CM log entry + non-zero exit code (task sequence marks step failed)
- Missing required TS vars → CM log entry + configurable abort-or-continue per action
- HTTP errors → CM log entry + action result carries error; downstream actions can branch on it
- Script errors → exit code captured and optionally written as a TS variable
- No silent failures — every error path produces at least one CM log entry

---

## Dependencies (vcpkg)

| Package | Purpose |
|---|---|
| `pugixml` | XML parsing — fast, header-friendly |
| `libcurl` | HTTP client (pinned 7.83.1 for parity with original) |
| `wil` | Win32 RAII wrappers (smart handles, error helpers) |
| `nlohmann-json` | REST response parsing |
| `catch2` | Unit testing framework |

---

## Testing Strategy

- **Catch2** unit tests in `tests/` target `osdui-core` only — no Win32 required to build or run
- Each action tested with `MapVariableStore` + `ScriptedDialogPresenter` mocks
- XML parsing tested against fixture `.xml` files in `tests/fixtures/`
- HTTP client tested against recorded response fixtures
- UI layer (`app/`) covered by manual OSD testing — it is intentionally thin

---

## Build

```bash
# Configure (vcpkg integration via CMakePresets)
cmake --preset windows-release

# Build
cmake --build --preset windows-release

# Test (core only, no Win32 required on dev machine)
ctest --preset windows-release
```

Targets: `Win32` and `x64`. CI via GitHub Actions on `windows-latest`.
