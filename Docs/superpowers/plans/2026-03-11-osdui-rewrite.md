# osdUI Rewrite Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete C++20 rewrite of UI++ as osdUI — a headless core static library + thin Win32 executable for SCCM OSD task sequences.

**Architecture:** `osdui-core` static lib (pure C++20, no Win32 UI) + `osdui.exe` (Win32+wil thin layer). Four interfaces (`IVariableStore`, `IDialogPresenter`, `IScriptHost`, `IConditionEvaluator`) form the testability boundary between core and platform. ActionRunner drives an ordered ActionGraph, calling back through interfaces.

**Tech Stack:** C++20, CMake + vcpkg, Win32+wil (UI layer), pugixml (XML), libcurl (HTTP), nlohmann-json (JSON), Catch2 (tests), GitHub Actions CI.

**Spec:** `Docs/superpowers/specs/2026-03-11-osdui-modernization-design.md`

---

## File Structure

```
osdUI/
├── CMakeLists.txt                          # root: declares both targets
├── CMakePresets.json                       # windows-release, windows-debug presets
├── vcpkg.json                              # updated: pugixml, wil, nlohmann-json, catch2 added
├── .github/workflows/ci.yml               # updated: cmake + ctest
├── core/
│   ├── CMakeLists.txt                      # osdui-core target
│   ├── include/osdui/
│   │   ├── interfaces.hpp                  # IVariableStore, IDialogPresenter, IScriptHost, IConditionEvaluator
│   │   ├── model.hpp                       # DialogSpec, ActionSpec, InputSpec, DialogResult, ScriptResult
│   │   └── action_graph.hpp               # ActionGraph, ActionNode, IAction
│   └── src/
│       ├── logging/
│       │   ├── cm_log.hpp
│       │   └── cm_log.cpp                  # CMTrace-compatible log writer
│       ├── config/
│       │   ├── config_parser.hpp
│       │   └── config_parser.cpp           # pugixml → ActionGraph
│       ├── actions/
│       │   ├── action_runner.hpp
│       │   ├── action_runner.cpp           # executes ActionGraph via interfaces
│       │   ├── action_default_values.hpp/.cpp
│       │   ├── action_external_call.hpp/.cpp
│       │   ├── action_reg_read.hpp/.cpp
│       │   ├── action_reg_write.hpp/.cpp
│       │   ├── action_wmi_read.hpp/.cpp
│       │   ├── action_wmi_write.hpp/.cpp
│       │   ├── action_file_read.hpp/.cpp
│       │   ├── action_switch.hpp/.cpp
│       │   ├── action_random_string.hpp/.cpp
│       │   ├── action_software_discovery.hpp/.cpp
│       │   ├── action_match.hpp/.cpp
│       │   ├── action_rest.hpp/.cpp
│       │   ├── action_user_input.hpp/.cpp
│       │   ├── action_user_info.hpp/.cpp
│       │   ├── action_preflight.hpp/.cpp
│       │   ├── action_app_tree.hpp/.cpp
│       │   ├── action_ts_var.hpp/.cpp
│       │   ├── action_ts_var_list.hpp/.cpp
│       │   ├── action_save_items.hpp/.cpp
│       │   ├── action_error_info.hpp/.cpp
│       │   └── action_vars.hpp/.cpp
│       ├── http/
│       │   ├── http_client.hpp
│       │   └── http_client.cpp             # curl wrapper
│       └── script/
│           ├── condition_evaluator.hpp
│           └── condition_evaluator.cpp     # expression parser; WSH fallback
├── app/
│   ├── CMakeLists.txt                      # osdui target
│   ├── main.cpp                            # entry point
│   ├── platform/
│   │   ├── ts_variables.hpp/.cpp           # COM ITSEnvClass → IVariableStore
│   │   └── wsh_script_host.hpp/.cpp        # WSH/CreateProcess → IScriptHost
│   └── dialogs/
│       ├── dialog_presenter.hpp/.cpp       # Win32 IDialogPresenter impl
│       ├── dialog_user_input.hpp/.cpp
│       ├── dialog_user_info.hpp/.cpp
│       ├── dialog_preflight.hpp/.cpp
│       ├── dialog_app_tree.hpp/.cpp
│       ├── dialog_ts_var.hpp/.cpp
│       ├── dialog_vars.hpp/.cpp
│       ├── dialog_error_info.hpp/.cpp
│       └── dialog_save_items.hpp/.cpp
└── tests/
    ├── CMakeLists.txt                      # osdui-tests target
    ├── fixtures/
    │   ├── basic.xml
    │   ├── switch.xml
    │   └── preflight.xml
    ├── mocks/
    │   ├── mock_variable_store.hpp
    │   ├── mock_dialog_presenter.hpp
    │   ├── mock_script_host.hpp
    │   └── mock_condition_evaluator.hpp
    ├── test_config_parser.cpp
    ├── test_action_runner.cpp
    ├── test_action_default_values.cpp
    ├── test_action_switch.cpp
    ├── test_action_random_string.cpp
    ├── test_action_file_read.cpp
    └── test_condition_evaluator.cpp
```

---

## Chunk 1: Project Skeleton

Sets up the CMake build system, updates vcpkg dependencies, and creates the directory skeleton. Produces a solution that configures and builds (even with empty source files) before any logic is written.

### Task 1: Update vcpkg.json

**Files:**
- Modify: `vcpkg.json`

- [ ] **Step 1: Replace vcpkg.json content**

```json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "dependencies": [
    "curl",
    "pugixml",
    "wil",
    "nlohmann-json",
    "catch2"
  ],
  "overrides": [
    {
      "name": "curl",
      "version": "7.83.1"
    }
  ],
  "vcpkg-configuration": {
    "default-registry": {
      "kind": "git",
      "baseline": "2dc91c6439568f694052c3fa25859dc78d9ff8e4",
      "repository": "https://github.com/microsoft/vcpkg"
    },
    "registries": [
      {
        "kind": "artifact",
        "location": "https://github.com/microsoft/vcpkg-ce-catalog/archive/refs/heads/main.zip",
        "name": "microsoft"
      }
    ]
  }
}
```

- [ ] **Step 2: Commit**

```bash
git add vcpkg.json
git commit -m "chore: add pugixml, wil, nlohmann-json, catch2 to vcpkg"
```

---

### Task 2: Root CMakeLists.txt

**Files:**
- Create: `CMakeLists.txt` (replaces old .sln-based build)

- [ ] **Step 1: Write root CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.25)
project(osdUI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static CRT for WinPE compatibility
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(pugixml CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(WIL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(core)
add_subdirectory(app)

enable_testing()
add_subdirectory(tests)
```

- [ ] **Step 2: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: add root CMakeLists.txt"
```

---

### Task 3: CMakePresets.json

**Files:**
- Create: `CMakePresets.json`

- [ ] **Step 1: Write CMakePresets.json**

Note: Visual Studio is a multi-config generator — `CMAKE_BUILD_TYPE` is ignored. Configuration is selected at build time via `"configuration"` in `buildPresets`.

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "windows-base",
      "hidden": true,
      "generator": "Visual Studio 17 2022",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "cacheVariables": {
        "VCPKG_TARGET_TRIPLET": "x64-windows-static"
      }
    },
    {
      "name": "windows-release",
      "inherits": "windows-base",
      "displayName": "Windows Release (x64)"
    },
    {
      "name": "windows-debug",
      "inherits": "windows-base",
      "displayName": "Windows Debug (x64)"
    }
  ],
  "buildPresets": [
    {
      "name": "windows-release",
      "configurePreset": "windows-release",
      "configuration": "Release"
    },
    {
      "name": "windows-debug",
      "configurePreset": "windows-debug",
      "configuration": "Debug"
    }
  ],
  "testPresets": [
    {
      "name": "windows-release",
      "configurePreset": "windows-release",
      "configuration": "Release",
      "output": { "outputOnFailure": true }
    }
  ]
}
```

- [ ] **Step 2: Commit**

```bash
git add CMakePresets.json
git commit -m "build: add CMakePresets.json"
```

---

### Task 4: Core CMakeLists.txt + directory skeleton

**Files:**
- Create: `core/CMakeLists.txt`
- Create: `core/include/osdui/.gitkeep`
- Create: `core/src/logging/.gitkeep`
- Create: `core/src/config/.gitkeep`
- Create: `core/src/actions/.gitkeep`
- Create: `core/src/http/.gitkeep`
- Create: `core/src/script/.gitkeep`

- [ ] **Step 1: Create directory tree**

```bash
mkdir -p core/include/osdui
mkdir -p core/src/logging core/src/config core/src/actions core/src/http core/src/script
mkdir -p app/platform app/dialogs
mkdir -p tests/mocks tests/fixtures
```

- [ ] **Step 2: Write core/CMakeLists.txt**

```cmake
add_library(osdui-core STATIC)

target_include_directories(osdui-core
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(osdui-core
  PUBLIC  pugixml::pugixml
  PRIVATE CURL::libcurl
          nlohmann_json::nlohmann_json
)

# Glob sources — add each file explicitly as you create it
target_sources(osdui-core PRIVATE
  # populated in later tasks
)
```

- [ ] **Step 3: Write app/CMakeLists.txt**

```cmake
add_executable(osdui WIN32)

target_sources(osdui PRIVATE
  main.cpp
)

target_link_libraries(osdui
  PRIVATE osdui-core
          Microsoft::WIL
)

# populated in later tasks
```

- [ ] **Step 4: Write a stub app/main.cpp so it compiles**

```cpp
#include <windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    return 0;
}
```

- [ ] **Step 5: Write tests/CMakeLists.txt and stub test**

```cmake
find_package(Catch2 3 CONFIG REQUIRED)

add_executable(osdui-tests
  test_stub.cpp   # replaced by real tests in Chunk 2+
)

target_link_libraries(osdui-tests
  PRIVATE osdui-core
          Catch2::Catch2WithMain
)

target_include_directories(osdui-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(CTest)
list(APPEND CMAKE_MODULE_PATH "${Catch2_DIR}/../../../share/catch2")
include(Catch)
catch_discover_tests(osdui-tests)
```

Create `tests/test_stub.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
TEST_CASE("placeholder") { SUCCEED("skeleton builds"); }
```

- [ ] **Step 6: Verify the skeleton configures and builds**

```bash
# In VS Developer Command Prompt with VCPKG_ROOT set to your vcpkg installation:
# e.g. set VCPKG_ROOT=C:\vcpkg
cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release --no-tests=error
```

Expected: configure succeeds, `osdui.exe` produced at `build/windows-release/app/Release/osdui.exe`, 1 test passes ("placeholder").

- [ ] **Step 7: Commit**

```bash
git add core/ app/ tests/
git commit -m "build: scaffold CMake project skeleton"
```

---

### Task 5: Update CI

**Files:**
- Modify: `.github/workflows/release.yml` → rename to `ci.yml`

- [ ] **Step 1: Replace workflow file**

```bash
mv .github/workflows/release.yml .github/workflows/ci.yml
```

Note: The rewrite targets x64 only (Win32/x86 is dropped — modern WinPE is x64). The original CI built both; this CI builds x64 Release only.

- [ ] **Step 2: Write new ci.yml**

```yaml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest

    env:
      VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"

    steps:
      - uses: actions/checkout@v4

      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11
        with:
          vcpkgGitCommitId: '2dc91c6439568f694052c3fa25859dc78d9ff8e4'

      - name: Setup vcpkg cache
        uses: actions/github-script@v7
        with:
          script: |
            core.exportVariable('ACTIONS_CACHE_URL', process.env.ACTIONS_CACHE_URL || '');
            core.exportVariable('ACTIONS_RUNTIME_TOKEN', process.env.ACTIONS_RUNTIME_TOKEN || '');

      - name: Configure
        run: cmake --preset windows-release

      - name: Build
        run: cmake --build --preset windows-release

      - name: Test
        run: ctest --preset windows-release --no-tests=error

      - uses: actions/upload-artifact@v4
        with:
          name: osdui-x64-release
          path: build/windows-release/app/Release/osdui.exe
```

- [ ] **Step 3: Commit**

```bash
git add .github/workflows/ci.yml
git rm .github/workflows/release.yml
git commit -m "ci: replace release.yml with CMake-based CI pipeline"
```

---

## Chunk 2: Core Interfaces, Model & Mocks

Defines all public headers and mock implementations. After this chunk, tests can be written for any action without touching the UI or platform layer.

### Task 6: Public headers — interfaces.hpp and model.hpp

**Files:**
- Create: `core/include/osdui/interfaces.hpp`
- Create: `core/include/osdui/model.hpp`
- Create: `core/include/osdui/action_graph.hpp`

- [ ] **Step 1: Write interfaces.hpp**

```cpp
#pragma once
#include <optional>
#include <string>
#include <map>
#include "model.hpp"

namespace osdui {

struct IVariableStore {
    virtual ~IVariableStore() = default;
    virtual std::optional<std::wstring> get(std::wstring_view name) const = 0;
    // Throws wil::ResultException on COM failure in production impl
    virtual void set(std::wstring_view name, std::wstring_view value) = 0;
};

struct IDialogPresenter {
    virtual ~IDialogPresenter() = default;
    virtual model::DialogResult present(const model::DialogSpec& spec,
                                        const IVariableStore& vars) = 0;
};

struct IScriptHost {
    virtual ~IScriptHost() = default;
    // language: empty/"exe" → CreateProcess; "vbscript"/"jscript" → WSH
    virtual model::ScriptResult execute(std::wstring_view language,
                                        std::wstring_view script) = 0;
};

struct IConditionEvaluator {
    virtual ~IConditionEvaluator() = default;
    virtual bool evaluate(std::wstring_view expression,
                          const IVariableStore& vars) const = 0;
};

} // namespace osdui
```

- [ ] **Step 2: Write model.hpp**

```cpp
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
```

- [ ] **Step 3: Write action_graph.hpp**

```cpp
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include "interfaces.hpp"

namespace osdui {

// ── IAction ──────────────────────────────────────────────────────────────────

struct ActionContext {
    IVariableStore&      vars;
    IDialogPresenter&    dialogs;
    IScriptHost&         scripts;
    IConditionEvaluator& conditions;
};

enum class ActionOutcome { Continue, Abort, JumpTo };

struct ActionResult {
    ActionOutcome outcome{ActionOutcome::Continue};
    std::wstring  jump_target;  // node id when outcome == JumpTo
};

struct IAction {
    virtual ~IAction() = default;
    virtual ActionResult execute(ActionContext& ctx) = 0;
    // Optional condition expression; empty = always run
    std::wstring condition;
};

// ── ActionGraph ──────────────────────────────────────────────────────────────

struct ActionNode {
    std::wstring          id;    // optional XML id= attribute
    std::unique_ptr<IAction> action;
};

struct ActionGraph {
    std::vector<ActionNode> nodes;

    // Returns index of node with given id, or npos if not found
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
    std::size_t find(std::wstring_view id) const noexcept {
        for (std::size_t i = 0; i < nodes.size(); ++i)
            if (nodes[i].id == id) return i;
        return npos;
    }
};

} // namespace osdui
```

- [ ] **Step 4: Add a compile-check test (no logic yet)**

Create `tests/test_interfaces_compile.cpp`:
```cpp
#include <osdui/interfaces.hpp>
#include <osdui/model.hpp>
#include <osdui/action_graph.hpp>

// If this file compiles, the public headers are well-formed.
TEST_CASE("headers compile") {
    SUCCEED("headers compiled successfully");
}
```

Add to `tests/CMakeLists.txt`:
```cmake
target_sources(osdui-tests PRIVATE
  test_interfaces_compile.cpp
)
```

- [ ] **Step 5: Build and run test**

```bash
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Expected: 1 test passes.

- [ ] **Step 6: Commit**

```bash
git add core/include/ tests/
git commit -m "feat: add public headers — interfaces, model, action_graph"
```

---

### Task 7: Mock implementations

**Files:**
- Create: `tests/mocks/mock_variable_store.hpp`
- Create: `tests/mocks/mock_dialog_presenter.hpp`
- Create: `tests/mocks/mock_script_host.hpp`
- Create: `tests/mocks/mock_condition_evaluator.hpp`

- [ ] **Step 1: Write mock_variable_store.hpp**

```cpp
#pragma once
#include <osdui/interfaces.hpp>
#include <map>
#include <stdexcept>

namespace osdui::test {

class MapVariableStore : public IVariableStore {
public:
    std::optional<std::wstring> get(std::wstring_view name) const override {
        auto it = store_.find(std::wstring{name});
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }
    void set(std::wstring_view name, std::wstring_view value) override {
        store_[std::wstring{name}] = std::wstring{value};
    }
    // Test helper: check a variable was set to a value
    bool has(std::wstring_view name, std::wstring_view value) const {
        auto v = get(name);
        return v && *v == value;
    }
private:
    std::map<std::wstring, std::wstring> store_;
};

} // namespace osdui::test
```

- [ ] **Step 2: Write mock_dialog_presenter.hpp**

```cpp
#pragma once
#include <osdui/interfaces.hpp>
#include <queue>
#include <stdexcept>

namespace osdui::test {

// Returns pre-queued results in order; throws if queue is exhausted.
class ScriptedDialogPresenter : public IDialogPresenter {
public:
    void enqueue(model::DialogResult result) {
        queue_.push(std::move(result));
    }
    model::DialogResult present(const model::DialogSpec&,
                                const IVariableStore&) override {
        if (queue_.empty()) throw std::logic_error{"ScriptedDialogPresenter: no more results"};
        auto r = std::move(queue_.front());
        queue_.pop();
        return r;
    }
private:
    std::queue<model::DialogResult> queue_;
};

} // namespace osdui::test
```

- [ ] **Step 3: Write mock_script_host.hpp**

```cpp
#pragma once
#include <osdui/interfaces.hpp>
#include <vector>

namespace osdui::test {

struct CapturedInvocation {
    std::wstring language;
    std::wstring script;
};

// Queue-based: enqueue expected results in order; throws if queue exhausted.
class CapturingScriptHost : public IScriptHost {
public:
    void enqueue(int exit_code, std::wstring output = {}) {
        results_.push({exit_code, std::move(output)});
    }
    model::ScriptResult execute(std::wstring_view language,
                                std::wstring_view script) override {
        invocations_.push_back({std::wstring{language}, std::wstring{script}});
        if (results_.empty()) return {0, {}};  // default: success, no output
        auto r = std::move(results_.front());
        results_.pop();
        return r;
    }
    const std::vector<CapturedInvocation>& invocations() const { return invocations_; }
private:
    std::vector<CapturedInvocation>       invocations_;
    std::queue<model::ScriptResult>       results_;
};

} // namespace osdui::test
```

- [ ] **Step 4: Write mock_condition_evaluator.hpp**

```cpp
#pragma once
#include <osdui/interfaces.hpp>
#include <map>

namespace osdui::test {

// Returns pre-configured bool for each expression.
// Throws std::logic_error for any expression not explicitly configured —
// this makes test omissions visible rather than silently passing.
class LiteralConditionEvaluator : public IConditionEvaluator {
public:
    void set(std::wstring_view expression, bool value) {
        table_[std::wstring{expression}] = value;
    }
    bool evaluate(std::wstring_view expression,
                  const IVariableStore&) const override {
        if (expression.empty()) return true;
        auto it = table_.find(std::wstring{expression});
        if (it == table_.end())
            throw std::logic_error{"LiteralConditionEvaluator: unconfigured expression"};
        return it->second;
    }
private:
    std::map<std::wstring, bool> table_;
};

} // namespace osdui::test
```

- [ ] **Step 5: Add compile-check for mocks**

Add to `tests/test_interfaces_compile.cpp`:
```cpp
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"

TEST_CASE("mocks compile") {
    osdui::test::MapVariableStore vars;
    osdui::test::ScriptedDialogPresenter dlg;
    osdui::test::CapturingScriptHost sh;
    osdui::test::LiteralConditionEvaluator ce;
    SUCCEED();
}
```

- [ ] **Step 6: Build and run tests**

```bash
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Expected: 2 tests pass.

- [ ] **Step 7: Commit**

```bash
git add tests/mocks/ tests/test_interfaces_compile.cpp
git commit -m "test: add mock implementations for all four interfaces"
```

---

## Chunk 3: Logging + ConfigParser

Implements CMTrace-compatible logging and XML config parsing. After this chunk, you can parse any existing UI++ config file into an ActionGraph.

### Task 8: CM Log writer

**Files:**
- Create: `core/src/logging/cm_log.hpp`
- Create: `core/src/logging/cm_log.cpp`

- [ ] **Step 1: Write cm_log.hpp**

```cpp
#pragma once
#include <string>
#include <filesystem>

namespace osdui::logging {

enum class Severity { Info, Warning, Error };

// Writes CMTrace-compatible .log entries.
// Thread-safe: each call opens, writes, and closes the file.
class CmLog {
public:
    explicit CmLog(std::filesystem::path path);

    void info   (std::wstring_view component, std::wstring_view message);
    void warning(std::wstring_view component, std::wstring_view message);
    void error  (std::wstring_view component, std::wstring_view message);

private:
    void write(Severity sev, std::wstring_view component, std::wstring_view message);
    std::filesystem::path path_;
};

} // namespace osdui::logging
```

- [ ] **Step 2: Write cm_log.cpp**

```cpp
#include "cm_log.hpp"
#include <windows.h>
#include <fstream>
#include <chrono>
#include <format>
#include <mutex>

namespace osdui::logging {

namespace {
std::mutex g_log_mutex;

std::string to_utf8(std::wstring_view ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(),
                                  nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(),
                        s.data(), len, nullptr, nullptr);
    return s;
}

int severity_to_int(Severity s) {
    switch (s) {
        case Severity::Info:    return 1;
        case Severity::Warning: return 2;
        case Severity::Error:   return 3;
    }
    return 1;
}
} // anon namespace

CmLog::CmLog(std::filesystem::path path) : path_{std::move(path)} {}

void CmLog::info   (std::wstring_view c, std::wstring_view m) { write(Severity::Info,    c, m); }
void CmLog::warning(std::wstring_view c, std::wstring_view m) { write(Severity::Warning, c, m); }
void CmLog::error  (std::wstring_view c, std::wstring_view m) { write(Severity::Error,   c, m); }

void CmLog::write(Severity sev, std::wstring_view component, std::wstring_view message) {
    using namespace std::chrono;
    auto now  = system_clock::now();
    auto tt   = system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_s(&tm, &tt);

    auto ms   = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // CMTrace log format:
    // <![LOG[message]LOG]!><time="HH:MM:SS.mmm+000" date="M-D-YYYY" component="comp" type=N thread="0" file="">
    std::string line = std::format(
        "<![LOG[{}]LOG]!><time=\"{:02d}:{:02d}:{:02d}.{:03d}+000\" "
        "date=\"{}-{}-{}\" component=\"{}\" type={} thread=\"0\" file=\"\">\n",
        to_utf8(message),
        tm.tm_hour, tm.tm_min, tm.tm_sec, (int)ms.count(),
        tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900,
        to_utf8(component),
        severity_to_int(sev)
    );

    std::lock_guard lock{g_log_mutex};
    std::ofstream f{path_, std::ios::app};
    f << line;
}

} // namespace osdui::logging
```

- [ ] **Step 3: Add cm_log.cpp to core/CMakeLists.txt**

```cmake
target_sources(osdui-core PRIVATE
  src/logging/cm_log.cpp
)
```

- [ ] **Step 4: Build to confirm it compiles**

```bash
cmake --build --preset windows-debug
```

Expected: builds cleanly.

- [ ] **Step 5: Commit**

```bash
git add core/src/logging/ core/CMakeLists.txt
git commit -m "feat: add CMTrace-compatible log writer"
```

---

### Task 9: ConfigParser

**Files:**
- Create: `core/src/config/config_parser.hpp`
- Create: `core/src/config/config_parser.cpp`
- Create: `tests/fixtures/basic.xml`
- Create: `tests/fixtures/switch.xml`
- Create: `tests/test_config_parser.cpp`

- [ ] **Step 1: Write the failing test first**

Create `tests/fixtures/basic.xml`:
```xml
<?xml version="1.0" encoding="utf-8"?>
<UIpp>
  <Actions>
    <Action Type="DefaultValues">
      <Variable Name="OSDComputerName" Value="DESKTOP" />
    </Action>
    <Action Type="Input" Title="Computer Name">
      <InputField Type="TextInput" Variable="OSDComputerName" Prompt="Computer Name" />
    </Action>
  </Actions>
</UIpp>
```

Add `FIXTURES_DIR` define to `tests/CMakeLists.txt` so tests can find fixture files regardless of working directory:

```cmake
target_compile_definitions(osdui-tests PRIVATE
  FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
```

Create `tests/test_config_parser.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <osdui/action_graph.hpp>
// ConfigParser not yet included — this will fail to compile first:
#include "../../core/src/config/config_parser.hpp"

// FIXTURES_DIR is defined by CMake as the absolute path to tests/fixtures/
static const std::filesystem::path fixtures{FIXTURES_DIR};

TEST_CASE("ConfigParser parses basic XML") {
    osdui::config::ConfigParser parser;
    auto graph = parser.parse(fixtures / "basic.xml");
    REQUIRE(graph.nodes.size() == 2);
}

TEST_CASE("ConfigParser: missing file throws") {
    osdui::config::ConfigParser parser;
    REQUIRE_THROWS(parser.parse(fixtures / "nonexistent.xml"));
}
```

- [ ] **Step 2: Add test file to CMakeLists.txt and verify it fails to compile**

```cmake
target_sources(osdui-tests PRIVATE
  test_interfaces_compile.cpp
  test_config_parser.cpp
)
```

```bash
cmake --build --preset windows-debug
```

Expected: compile error — `config_parser.hpp` not found.

- [ ] **Step 3: Write config_parser.hpp**

```cpp
#pragma once
#include <osdui/action_graph.hpp>
#include <filesystem>
#include <stdexcept>

namespace osdui::config {

struct ParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ConfigParser {
public:
    // Parses the XML config file at path and returns an ActionGraph.
    // Throws ParseError on malformed XML or missing file.
    // Unknown action types are logged as warnings and skipped.
    ActionGraph parse(const std::filesystem::path& path) const;
};

} // namespace osdui::config
```

- [ ] **Step 4: Write config_parser.cpp (skeleton that passes the tests)**

```cpp
#include "config_parser.hpp"
#include <pugixml.hpp>
#include <format>
#include <vector>

namespace osdui::config {
namespace {

// Stub: return a non-null placeholder for known types so tests pass.
// Real factories replace individual branches in Chunk 5+.
std::unique_ptr<IAction> make_action(std::wstring_view type, const pugi::xml_node& /*node*/) {
    static const std::vector<std::wstring> known_types = {
        L"DefaultValues", L"Input", L"Info", L"InfoFullScreen",
        L"AppTree", L"ExternalCall", L"Preflight", L"RegRead", L"RegWrite",
        L"WMIRead", L"WMIWrite", L"FileRead", L"Switch", L"TSVar",
        L"TSVarList", L"RandomString", L"SoftwareDiscovery", L"SaveItems",
        L"Match", L"ErrorInfo", L"Vars", L"Rest"
    };

    struct PlaceholderAction : IAction {
        ActionResult execute(ActionContext&) override { return {}; }
    };

    for (const auto& k : known_types)
        if (k == type) return std::make_unique<PlaceholderAction>();

    return nullptr;  // unknown type — caller skips with warning
}

} // anon namespace

ActionGraph ConfigParser::parse(const std::filesystem::path& path) const {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result)
        throw ParseError{std::format("Failed to parse '{}': {}",
            path.string(), result.description())};

    ActionGraph graph;
    auto actions_node = doc.child(L"UIpp").child(L"Actions");
    if (!actions_node)
        throw ParseError{"XML missing <UIpp><Actions> structure"};

    for (const auto& action_node : actions_node.children(L"Action")) {
        std::wstring type = action_node.attribute(L"Type").as_string();
        std::wstring id   = action_node.attribute(L"id").as_string();

        auto action = make_action(type, action_node);
        if (!action) continue;  // unknown type — skipped

        action->condition = action_node.attribute(L"Condition").as_string();
        graph.nodes.push_back({std::move(id), std::move(action)});
    }
    return graph;
}

} // namespace osdui::config
```

- [ ] **Step 5: Add config_parser.cpp to core/CMakeLists.txt**

```cmake
target_sources(osdui-core PRIVATE
  src/logging/cm_log.cpp
  src/config/config_parser.cpp
)
```

- [ ] **Step 6: Build and run tests**

```bash
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Expected: 3 tests pass (headers compile, mocks compile, ConfigParser basic + throws).

- [ ] **Step 7: Commit**

```bash
git add core/src/config/ tests/fixtures/ tests/test_config_parser.cpp core/CMakeLists.txt
git commit -m "feat: add ConfigParser — parses UI++ XML into ActionGraph"
```

---

## Chunk 4: ActionRunner + ConditionEvaluator

Drives the ActionGraph and evaluates conditions. After this chunk, you can write an XML config, parse it, and run it end-to-end in tests.

### Task 10: ConditionEvaluator

**Files:**
- Create: `core/src/script/condition_evaluator.hpp`
- Create: `core/src/script/condition_evaluator.cpp`
- Create: `tests/test_condition_evaluator.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/test_condition_evaluator.cpp
#include <catch2/catch_test_macros.hpp>
#include "../../core/src/script/condition_evaluator.hpp"
#include "mocks/mock_variable_store.hpp"

using namespace osdui;
using namespace osdui::test;

TEST_CASE("ConditionEvaluator: empty expression returns true") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    REQUIRE(eval.evaluate(L"", vars) == true);
}

TEST_CASE("ConditionEvaluator: literal true/false") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    REQUIRE(eval.evaluate(L"true",  vars) == true);
    REQUIRE(eval.evaluate(L"false", vars) == false);
    REQUIRE(eval.evaluate(L"1",     vars) == true);
    REQUIRE(eval.evaluate(L"0",     vars) == false);
}

TEST_CASE("ConditionEvaluator: variable substitution + equality") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    vars.set(L"OSDModel", L"HP EliteBook");
    // Condition format: %Variable% == "value"
    REQUIRE(eval.evaluate(L"%OSDModel% == \"HP EliteBook\"", vars) == true);
    REQUIRE(eval.evaluate(L"%OSDModel% == \"Dell XPS\"",     vars) == false);
}

TEST_CASE("ConditionEvaluator: variable substitution + not-equals") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    vars.set(L"OSDIsVM", L"false");
    REQUIRE(eval.evaluate(L"%OSDIsVM% != \"true\"", vars) == true);
}
```

- [ ] **Step 2: Add to tests/CMakeLists.txt and verify fail**

```cmake
target_sources(osdui-tests PRIVATE
  test_interfaces_compile.cpp
  test_config_parser.cpp
  test_condition_evaluator.cpp
)
```

```bash
cmake --build --preset windows-debug
```

Expected: compile error — `condition_evaluator.hpp` not found.

- [ ] **Step 3: Write condition_evaluator.hpp**

```cpp
#pragma once
#include <osdui/interfaces.hpp>

namespace osdui::script {

// Evaluates boolean expressions used in condition= attributes.
// Supports: empty (true), "true"/"false"/"1"/"0",
//           %VAR% == "value", %VAR% != "value"
// Unknown/complex expressions: returns true (permissive) and logs a warning.
class ConditionEvaluator : public IConditionEvaluator {
public:
    bool evaluate(std::wstring_view expression,
                  const IVariableStore& vars) const override;

private:
    std::wstring substitute_vars(std::wstring_view expr,
                                 const IVariableStore& vars) const;
};

} // namespace osdui::script
```

- [ ] **Step 4: Write condition_evaluator.cpp**

```cpp
#include "condition_evaluator.hpp"
#include <algorithm>
#include <regex>

namespace osdui::script {

std::wstring ConditionEvaluator::substitute_vars(
    std::wstring_view expr, const IVariableStore& vars) const
{
    std::wstring result{expr};
    std::wregex var_re{LR"(%([^%]+)%)"};
    std::wstring out;
    auto it = std::wsregex_iterator(result.begin(), result.end(), var_re);
    auto end = std::wsregex_iterator{};
    std::size_t last = 0;
    for (; it != end; ++it) {
        const auto& m = *it;
        out += result.substr(last, m.position() - last);
        auto val = vars.get(m[1].str());
        out += val ? *val : std::wstring{};
        last = m.position() + m.length();
    }
    out += result.substr(last);
    return out;
}

bool ConditionEvaluator::evaluate(
    std::wstring_view expression, const IVariableStore& vars) const
{
    if (expression.empty()) return true;

    std::wstring expr = substitute_vars(expression, vars);

    // Trim whitespace
    auto trim = [](std::wstring& s) {
        s.erase(0, s.find_first_not_of(L" \t"));
        s.erase(s.find_last_not_of(L" \t") + 1);
    };
    trim(expr);

    // Literal
    if (expr == L"true"  || expr == L"1") return true;
    if (expr == L"false" || expr == L"0") return false;

    // Equality: lhs == "rhs" or lhs != "rhs"
    std::wregex eq_re {LR"(^(.+?)\s*(==|!=)\s*\"(.*?)\"$)"};
    std::wsmatch m;
    if (std::regex_match(expr, m, eq_re)) {
        std::wstring lhs = m[1].str(); trim(lhs);
        std::wstring op  = m[2].str();
        std::wstring rhs = m[3].str();
        bool eq = (lhs == rhs);
        return (op == L"==") ? eq : !eq;
    }

    // Permissive fallback for complex expressions
    return true;
}

} // namespace osdui::script
```

- [ ] **Step 5: Add to core CMakeLists.txt and run tests**

```cmake
target_sources(osdui-core PRIVATE
  src/logging/cm_log.cpp
  src/config/config_parser.cpp
  src/script/condition_evaluator.cpp
)
```

```bash
cmake --build --preset windows-debug && ctest --preset windows-debug
```

Expected: all tests pass.

- [ ] **Step 6: Commit**

```bash
git add core/src/script/ tests/test_condition_evaluator.cpp core/CMakeLists.txt
git commit -m "feat: add ConditionEvaluator with variable substitution and equality ops"
```

---

### Task 11: ActionRunner

**Files:**
- Create: `core/src/actions/action_runner.hpp`
- Create: `core/src/actions/action_runner.cpp`
- Create: `tests/test_action_runner.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/test_action_runner.cpp
#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_runner.hpp"
#include "../../core/src/script/condition_evaluator.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"
#include <osdui/action_graph.hpp>

using namespace osdui;
using namespace osdui::test;

// A simple action that records its execution
struct RecordingAction : IAction {
    bool* executed;
    explicit RecordingAction(bool& flag) : executed{&flag} {}
    ActionResult execute(ActionContext&) override { *executed = true; return {}; }
};

TEST_CASE("ActionRunner executes all actions") {
    ActionGraph graph;
    bool a_ran = false, b_ran = false;
    graph.nodes.push_back({L"a", std::make_unique<RecordingAction>(a_ran)});
    graph.nodes.push_back({L"b", std::make_unique<RecordingAction>(b_ran)});

    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;

    actions::ActionRunner runner;
    auto result = runner.run(graph, vars, dlg, sh, ce);

    REQUIRE(result == actions::RunResult::Success);
    REQUIRE(a_ran);
    REQUIRE(b_ran);
}

TEST_CASE("ActionRunner skips action with false condition") {
    ActionGraph graph;
    bool ran = false;
    auto action = std::make_unique<RecordingAction>(ran);
    action->condition = L"ShouldNotRun";
    graph.nodes.push_back({L"", std::move(action)});

    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    ce.set(L"ShouldNotRun", false);

    actions::ActionRunner runner;
    runner.run(graph, vars, dlg, sh, ce);
    REQUIRE_FALSE(ran);
}

TEST_CASE("ActionRunner handles JumpTo — skips intervening nodes") {
    ActionGraph graph;
    bool skipped_ran = false;
    bool target_ran  = false;

    struct JumpAction : IAction {
        ActionResult execute(ActionContext&) override {
            return {ActionOutcome::JumpTo, L"target"};
        }
    };
    struct TargetAction : IAction {
        bool* ran;
        explicit TargetAction(bool& f) : ran{&f} {}
        ActionResult execute(ActionContext&) override { *ran = true; return {}; }
    };

    graph.nodes.push_back({L"jumper",  std::make_unique<JumpAction>()});
    graph.nodes.push_back({L"skipped", std::make_unique<RecordingAction>(skipped_ran)});
    graph.nodes.push_back({L"target",  std::make_unique<TargetAction>(target_ran)});

    MapVariableStore vars; ScriptedDialogPresenter dlg;
    CapturingScriptHost sh; LiteralConditionEvaluator ce;
    actions::ActionRunner runner;
    runner.run(graph, vars, dlg, sh, ce);

    REQUIRE_FALSE(skipped_ran);
    REQUIRE(target_ran);
}

- [ ] **Step 2: Write action_runner.hpp**

```cpp
#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/interfaces.hpp>
#include "../logging/cm_log.hpp"

namespace osdui::actions {

enum class RunResult { Success, Aborted };

class ActionRunner {
public:
    // log is optional — pass nullptr to disable logging (useful in tests)
    RunResult run(const ActionGraph&   graph,
                  IVariableStore&      vars,
                  IDialogPresenter&    dialogs,
                  IScriptHost&         scripts,
                  IConditionEvaluator& conditions,
                  logging::CmLog*      log = nullptr);
};

} // namespace osdui::actions
```

- [ ] **Step 3: Write action_runner.cpp**

```cpp
#include "action_runner.hpp"
#include <format>

namespace osdui::actions {

RunResult ActionRunner::run(const ActionGraph&   graph,
                            IVariableStore&      vars,
                            IDialogPresenter&    dialogs,
                            IScriptHost&         scripts,
                            IConditionEvaluator& conditions,
                            logging::CmLog*      log)
{
    ActionContext ctx{vars, dialogs, scripts, conditions};
    std::size_t i = 0;

    while (i < graph.nodes.size()) {
        const auto& node = graph.nodes[i];

        // Evaluate action-level condition
        if (!node.action->condition.empty()) {
            if (!conditions.evaluate(node.action->condition, vars)) {
                ++i;
                continue;
            }
        }

        ActionResult result = node.action->execute(ctx);

        switch (result.outcome) {
            case ActionOutcome::Continue:
                ++i;
                break;
            case ActionOutcome::Abort:
                return RunResult::Aborted;
            case ActionOutcome::JumpTo: {
                auto target = graph.find(result.jump_target);
                if (target == ActionGraph::npos) {
                    if (log) log->error(L"ActionRunner",
                        std::format(L"JumpTo target '{}' not found — aborting",
                                    result.jump_target));
                    return RunResult::Aborted;
                }
                i = target;
                break;
            }
        }
    }
    return RunResult::Success;
}

} // namespace osdui::actions
```

- [ ] **Step 4: Add to core and tests CMakeLists.txt and run tests**

```cmake
# core/CMakeLists.txt
target_sources(osdui-core PRIVATE
  src/logging/cm_log.cpp
  src/config/config_parser.cpp
  src/script/condition_evaluator.cpp
  src/actions/action_runner.cpp
)

# tests/CMakeLists.txt — add test source
target_sources(osdui-tests PRIVATE
  test_interfaces_compile.cpp
  test_config_parser.cpp
  test_condition_evaluator.cpp
  test_action_runner.cpp
)
```

```bash
cmake --build --preset windows-debug && ctest --preset windows-debug
```

Expected: all tests pass.

- [ ] **Step 5: Commit**

```bash
git add core/src/actions/action_runner.hpp core/src/actions/action_runner.cpp \
        tests/test_action_runner.cpp core/CMakeLists.txt tests/CMakeLists.txt
git commit -m "feat: add ActionRunner with condition evaluation and JumpTo support"
```

---

## Chunk 5: Non-UI Actions

Implements all actions that do not require a dialog — they just read/write variables, registry, WMI, files, etc. Each follows the same pattern: test → implement → commit.

> **Pattern for every action in this chunk:**
> 1. Write failing test
> 2. Write header + implementation
> 3. Register in `config_parser.cpp` `make_action()`
> 4. Add `.cpp` to `core/CMakeLists.txt`
> 5. Run tests
> 6. Commit

### Task 12: DefaultValues action

**Files:**
- Create: `core/src/actions/action_default_values.hpp/.cpp`
- Create: `tests/test_action_default_values.cpp`

XML format:
```xml
<Action Type="DefaultValues">
  <Variable Name="OSDComputerName" Value="DESKTOP" />
  <Variable Name="OSDSiteCode" Value="P01" />
</Action>
```

- [ ] **Step 1: Write failing test**

```cpp
// tests/test_action_default_values.cpp
#include <catch2/catch_test_macros.hpp>
#include "../../core/src/actions/action_default_values.hpp"
#include "mocks/mock_variable_store.hpp"
#include "mocks/mock_dialog_presenter.hpp"
#include "mocks/mock_script_host.hpp"
#include "mocks/mock_condition_evaluator.hpp"

using namespace osdui;
using namespace osdui::test;

static ActionContext make_ctx(IVariableStore& v, IDialogPresenter& d,
                               IScriptHost& s, IConditionEvaluator& c) {
    return {v, d, s, c};
}

TEST_CASE("DefaultValues: sets variables") {
    MapVariableStore vars;
    ScriptedDialogPresenter dlg;
    CapturingScriptHost sh;
    LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::DefaultValuesAction action;
    action.add(L"OSDComputerName", L"DESKTOP");
    action.add(L"OSDSiteCode",     L"P01");

    auto result = action.execute(ctx);

    REQUIRE(result.outcome == ActionOutcome::Continue);
    REQUIRE(vars.has(L"OSDComputerName", L"DESKTOP"));
    REQUIRE(vars.has(L"OSDSiteCode",     L"P01"));
}

TEST_CASE("DefaultValues: does not overwrite existing variable") {
    MapVariableStore vars;
    vars.set(L"OSDComputerName", L"EXISTING");
    ScriptedDialogPresenter dlg; CapturingScriptHost sh; LiteralConditionEvaluator ce;
    auto ctx = make_ctx(vars, dlg, sh, ce);

    actions::DefaultValuesAction action;
    action.add(L"OSDComputerName", L"DEFAULT");

    action.execute(ctx);
    // DefaultValues only sets if not already set
    REQUIRE(vars.has(L"OSDComputerName", L"EXISTING"));
}
```

- [ ] **Step 2: Write action_default_values.hpp**

```cpp
#pragma once
#include <osdui/action_graph.hpp>
#include <vector>
#include <utility>

namespace osdui::actions {

class DefaultValuesAction : public IAction {
public:
    void add(std::wstring name, std::wstring value) {
        defaults_.emplace_back(std::move(name), std::move(value));
    }
    ActionResult execute(ActionContext& ctx) override;
private:
    std::vector<std::pair<std::wstring, std::wstring>> defaults_;
};

} // namespace osdui::actions
```

- [ ] **Step 3: Write action_default_values.cpp**

```cpp
#include "action_default_values.hpp"

namespace osdui::actions {

ActionResult DefaultValuesAction::execute(ActionContext& ctx) {
    for (const auto& [name, value] : defaults_) {
        if (!ctx.vars.get(name).has_value())
            ctx.vars.set(name, value);
    }
    return {};
}

} // namespace osdui::actions
```

- [ ] **Step 4: Register in config_parser.cpp**

In `make_action()`, replace the `PlaceholderAction` stub for `L"DefaultValues"`:

```cpp
#include "../actions/action_default_values.hpp"

// In make_action():
if (type == L"DefaultValues") {
    auto action = std::make_unique<actions::DefaultValuesAction>();
    for (const auto& var : node.children(L"Variable")) {
        action->add(var.attribute(L"Name").as_string(),
                    var.attribute(L"Value").as_string());
    }
    return action;
}
```

- [ ] **Step 5: Add to core/CMakeLists.txt, build, test, commit**

```bash
cmake --build --preset windows-debug && ctest --preset windows-debug
git add core/src/actions/action_default_values.hpp \
        core/src/actions/action_default_values.cpp \
        core/src/config/config_parser.cpp \
        core/CMakeLists.txt \
        tests/test_action_default_values.cpp
git commit -m "feat: add DefaultValues action"
```

---

### Task 13: ExternalCall action

XML format:
```xml
<Action Type="ExternalCall" Run="cmd.exe /c echo hello" SuccessExitCode="0" Variable="OSDResult" />
```

Follow the same TDD pattern. Key behavior:
- `Run` attribute: command line to execute (via `IScriptHost` with `language=L"exe"`)
- `SuccessExitCode`: exit code treated as success (default 0)
- `Variable`: optional TS var to store exit code

Test: mock `CapturingScriptHost` with `next_exit_code = 0`, verify variable is set.

Create: `core/src/actions/action_external_call.hpp/.cpp`, `tests/test_action_external_call.cpp`

- [ ] **Step 1: Write failing test, header, implementation, register, build, test, commit** (follow Task 12 pattern)

---

### Task 14: Switch action

XML format:
```xml
<Action Type="Switch" Variable="OSDModel">
  <Case Value="HP EliteBook" GoTo="hp_step" />
  <Default GoTo="generic_step" />
</Action>
```

Key behavior:
- Reads a TS variable
- Returns `ActionOutcome::JumpTo` with the matching `GoTo` id
- Falls through to `Default` if no match
- Returns `Continue` if no match and no Default

Create: `core/src/actions/action_switch.hpp/.cpp`, `tests/test_action_switch.cpp`

- [ ] **Step 1–5: TDD pattern, register in config_parser, commit**

---

### Task 15: RandomString action

XML format:
```xml
<Action Type="RandomString" Variable="OSDPassword" Length="12" CharSet="alphanumeric" />
```

Key behavior: generates a random string of the specified length into `Variable`. CharSet: `alphanumeric`, `alpha`, `numeric`, `all`.

Create: `core/src/actions/action_random_string.hpp/.cpp`, `tests/test_action_random_string.cpp`

- [ ] **Step 1–5: TDD pattern, commit**

---

### Task 16: RegRead / RegWrite actions

XML format:
```xml
<Action Type="RegRead" Hive="HKLM" Key="SOFTWARE\OSD" Value="Version" Variable="OSDVersion" />
<Action Type="RegWrite" Hive="HKLM" Key="SOFTWARE\OSD" Value="Deployed" Data="true" Type="REG_SZ" />
```

Note: Both use Win32 registry APIs (`RegOpenKeyExW`, `RegQueryValueExW`, `RegSetValueExW`). These ARE Win32 but not UI — they belong in `osdui-core`. Test with real registry keys in a test-only hive, or abstract behind an `IRegistry` interface if needed.

Create: `core/src/actions/action_reg_read.hpp/.cpp`, `core/src/actions/action_reg_write.hpp/.cpp`

- [ ] **Step 1–5: TDD pattern (test against real registry or use a temp key), commit**

---

### Task 17: WMIRead / WMIWrite actions

XML format:
```xml
<Action Type="WMIRead" Query="SELECT Manufacturer FROM Win32_ComputerSystem" Property="Manufacturer" Variable="OSDManufacturer" />
```

Note: Uses COM WMI (`IWbemServices`). Include `<wbemidl.h>` and link `wbemuuid.lib`. Test with a known-stable WMI class (`Win32_OperatingSystem.Caption`).

Create: `core/src/actions/action_wmi_read.hpp/.cpp`, `core/src/actions/action_wmi_write.hpp/.cpp`

- [ ] **Step 1–5: TDD pattern, commit**

---

### Task 18: FileRead action

XML format:
```xml
<Action Type="FileRead" Path="C:\OSD\config.txt" Variable="OSDConfig" />
```

Key behavior: reads entire file contents (UTF-8 or UTF-16) into the TS variable.

Create: `core/src/actions/action_file_read.hpp/.cpp`, `tests/test_action_file_read.cpp`

- [ ] **Step 1–5: TDD pattern (write a temp file in the test), commit**

---

### Task 19: SoftwareDiscovery, Match, TSVar, TSVarList, SaveItems, ErrorInfo, Vars

Each follows the same TDD pattern. Create one `.hpp/.cpp` per action type, register in `config_parser.cpp`, add to `core/CMakeLists.txt`.

These actions are lower complexity (mostly read/write vars or delegate to IDialogPresenter). Key behaviors:

- **SoftwareDiscovery**: reads installed software from `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall` into TS vars
- **Match**: compares a TS var value against a list of patterns, sets an output variable
- **TSVar**: passes a variable list spec to `IDialogPresenter` for display (debug)
- **TSVarList**: list operations on TS var arrays (append, remove, count)
- **SaveItems**: writes selected software items to a file path
- **ErrorInfo**: passes an error spec to `IDialogPresenter` for display
- **Vars**: passes all current vars to `IDialogPresenter` for display

- [ ] **Step 1–5 per action: TDD, register, commit each separately**

---

## Chunk 6: UI-Presenting Actions + HTTP Client

Implements actions that require `IDialogPresenter` (core side only — no Win32 yet) and the REST action.

### Task 20: HTTP client

**Files:**
- Create: `core/src/http/http_client.hpp`
- Create: `core/src/http/http_client.cpp`

- [ ] **Step 1: Write http_client.hpp**

```cpp
#pragma once
#include <string>
#include <map>
#include <stdexcept>

namespace osdui::http {

struct HttpResponse {
    int         status_code{0};
    std::string body;
    bool        ok() const { return status_code >= 200 && status_code < 300; }
};

struct HttpError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpResponse get (const std::string& url,
                      const std::map<std::string, std::string>& headers = {});
    HttpResponse post(const std::string& url,
                      const std::string& body,
                      const std::map<std::string, std::string>& headers = {});

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace osdui::http
```

- [ ] **Step 2: Implement using libcurl (pimpl)**

```cpp
#include "http_client.hpp"
#include <curl/curl.h>
#include <stdexcept>

namespace osdui::http {

namespace {
std::size_t write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
    auto* buf = static_cast<std::string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}
} // anon

struct HttpClient::Impl {
    CURL* curl{nullptr};
    Impl()  { curl = curl_easy_init(); }
    ~Impl() { if (curl) curl_easy_cleanup(curl); }
};

HttpClient::HttpClient() : impl_{std::make_unique<Impl>()} {
    if (!impl_->curl) throw HttpError{"curl_easy_init failed"};
}
HttpClient::~HttpClient() = default;

HttpResponse HttpClient::get(const std::string& url,
                             const std::map<std::string, std::string>& headers)
{
    auto* curl = impl_->curl;
    curl_easy_reset(curl);
    HttpResponse resp;
    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &resp.body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_slist* hdrs = nullptr;
    for (const auto& [k, v] : headers)
        hdrs = curl_slist_append(hdrs, (k + ": " + v).c_str());
    if (hdrs) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

    CURLcode rc = curl_easy_perform(curl);
    if (hdrs) curl_slist_free_all(hdrs);
    if (rc != CURLE_OK) throw HttpError{curl_easy_strerror(rc)};

    long code{0};
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    resp.status_code = static_cast<int>(code);
    return resp;
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body,
                              const std::map<std::string, std::string>& headers)
{
    auto* curl = impl_->curl;
    curl_easy_reset(curl);
    HttpResponse resp;
    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,  (long)body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &resp.body);

    curl_slist* hdrs = nullptr;
    for (const auto& [k, v] : headers)
        hdrs = curl_slist_append(hdrs, (k + ": " + v).c_str());
    if (hdrs) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

    CURLcode rc = curl_easy_perform(curl);
    if (hdrs) curl_slist_free_all(hdrs);
    if (rc != CURLE_OK) throw HttpError{curl_easy_strerror(rc)};

    long code{0};
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    resp.status_code = static_cast<int>(code);
    return resp;
}

} // namespace osdui::http
```

- [ ] **Step 3: Add to CMakeLists and build**

```bash
cmake --build --preset windows-debug
git add core/src/http/ core/CMakeLists.txt
git commit -m "feat: add curl-based HTTP client"
```

---

### Task 21: REST action

XML format:
```xml
<Action Type="Rest" Url="https://api.example.com/osd" Method="GET"
        Variable="OSDResult" JsonPath="$.data.value" />
```

Key behavior: calls `HttpClient`, parses JSON response with nlohmann-json, extracts `JsonPath` value into TS variable.

Create: `core/src/actions/action_rest.hpp/.cpp`

- [ ] **Step 1–5: TDD pattern (mock HTTP with a fixture file), register, commit**

---

### Task 22: UserInput, UserInfo, InfoFullScreen, Preflight, AppTree actions (core side)

These actions build a `model::DialogSpec` and call `IDialogPresenter::present()`. The Win32 dialog is wired in Chunk 7. For now, tests use `ScriptedDialogPresenter`.

**UserInput** — XML:
```xml
<Action Type="Input" Title="System Info">
  <InputField Type="TextInput" Variable="OSDComputerName" Prompt="Computer Name" />
  <InputField Type="DropDownList" Variable="OSDSiteCode" Prompt="Site">
    <Option Value="P01" Text="Primary" />
  </InputField>
</Action>
```

Key behavior: build `DialogSpec` from XML, call `presenter.present()`, write returned `values` into TS vars.

Follow TDD pattern for each. Create files, register in `config_parser.cpp`, add to `core/CMakeLists.txt`, commit each.

- [ ] **Task 22a: UserInput action — TDD, register, commit**
- [ ] **Task 22b: UserInfo / InfoFullScreen actions — TDD, register, commit**
- [ ] **Task 22c: Preflight action — TDD, register, commit**
- [ ] **Task 22d: AppTree action — TDD, register, commit**

---

## Chunk 7: Platform Layer + Win32 Dialogs + main.cpp

Wires the real Win32 platform into the core. After this chunk you have a working `osdui.exe`.

### Task 23: TsVariables (COM IVariableStore)

**Files:**
- Create: `app/platform/ts_variables.hpp/.cpp`

- [ ] **Step 1: Write ts_variables.hpp**

```cpp
#pragma once
#include <osdui/interfaces.hpp>
#include <wil/com.h>

namespace osdui::platform {

// IVariableStore backed by the SCCM task sequence COM environment.
// Throws wil::ResultException if COM calls fail.
class TsVariables : public IVariableStore {
public:
    TsVariables();   // CoInitializes and binds ITSEnvClass
    ~TsVariables();

    std::optional<std::wstring> get(std::wstring_view name) const override;
    void set(std::wstring_view name, std::wstring_view value) override;

private:
    wil::com_ptr<IDispatch> ts_env_;
};

} // namespace osdui::platform
```

- [ ] **Step 2: Implement using ITSEnvClass COM object**

The `ITSEnvClass` progid is `Microsoft.SMS.TSEnvironment`. Use `CoCreateInstance` and `IDispatch` calls. Reference the original `CTSVar.cpp` in `UI++/TSVar.cpp` for the COM invocation pattern.

- [ ] **Step 3: Add to app/CMakeLists.txt, build, commit**

---

### Task 24: WshScriptHost

**Files:**
- Create: `app/platform/wsh_script_host.hpp/.cpp`

Implements `IScriptHost`:
- `language == L"exe"` or empty: use `CreateProcessW`
- Otherwise: use `IActiveScript` / `IActiveScriptParse` (WSH)

Reference `UI++/ScriptHost.cpp` for the original WSH COM pattern.

- [ ] **Step 1–3: Write, add to CMakeLists, build, commit**

---

### Task 25: Win32 DialogPresenter

**Files:**
- Create: `app/dialogs/dialog_presenter.hpp/.cpp`
- Create: `app/dialogs/dialog_user_input.hpp/.cpp`
- Create: `app/dialogs/dialog_user_info.hpp/.cpp`
- Create: `app/dialogs/dialog_preflight.hpp/.cpp`
- Create: `app/dialogs/dialog_app_tree.hpp/.cpp`
- Create: `app/dialogs/dialog_ts_var.hpp/.cpp`
- Create: `app/dialogs/dialog_vars.hpp/.cpp`
- Create: `app/dialogs/dialog_error_info.hpp/.cpp`

`DialogPresenter` dispatches to the correct Win32 dialog based on `DialogSpec` type. Each dialog is a Win32 `DialogBox` call. Reference the originals in `UI++/Dialogs/` for the dialog resource IDs, control layout, and message handling patterns.

Pattern per dialog:
1. Create `.rc` resource entry for the dialog template
2. Implement `DialogBoxW` with a `DLGPROC` message handler
3. On `WM_INITDIALOG`: populate controls from `DialogSpec`
4. On `IDOK`: harvest control values into `DialogResult::values`
5. On `IDCANCEL`: return `DialogResult{false, {}}`

- [ ] **Step 1–N: Implement each dialog, add to app/CMakeLists.txt, build, commit after each dialog**

---

### Task 26: main.cpp

**Files:**
- Modify: `app/main.cpp`

- [ ] **Step 1: Write the full entry point**

```cpp
#include <windows.h>
#include <wil/result.h>
#include <osdui/action_graph.hpp>
#include "../core/src/config/config_parser.hpp"
#include "../core/src/actions/action_runner.hpp"
#include "../core/src/script/condition_evaluator.hpp"
#include "../core/src/logging/cm_log.hpp"
#include "platform/ts_variables.hpp"
#include "platform/wsh_script_host.hpp"
#include "dialogs/dialog_presenter.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int)
{
    // Parse command line: /config:<path> /log:<path>
    std::wstring config_path = L"UI++.xml";
    std::wstring log_path    = L"osdui.log";

    // Simple arg parsing (extend as needed)
    std::wstring cmdline{lpCmdLine};
    auto get_arg = [&](std::wstring_view prefix) -> std::optional<std::wstring> {
        auto pos = cmdline.find(prefix);
        if (pos == std::wstring::npos) return std::nullopt;
        pos += prefix.size();
        auto end = cmdline.find(L' ', pos);
        return cmdline.substr(pos, end == std::wstring::npos ? end : end - pos);
    };
    if (auto p = get_arg(L"/config:")) config_path = *p;
    if (auto p = get_arg(L"/log:"))    log_path    = *p;

    osdui::logging::CmLog log{log_path};
    log.info(L"osdUI", L"Starting osdUI");

    try {
        osdui::config::ConfigParser parser;
        auto graph = parser.parse(config_path);

        osdui::platform::TsVariables     vars;
        osdui::platform::WshScriptHost   scripts;
        osdui::dialogs::DialogPresenter  dialogs{hInstance};
        osdui::script::ConditionEvaluator conditions;

        osdui::actions::ActionRunner runner;
        auto result = runner.run(graph, vars, dialogs, scripts, conditions);

        log.info(L"osdUI", result == osdui::actions::RunResult::Success
                            ? L"Completed successfully" : L"Aborted");
        return result == osdui::actions::RunResult::Success ? 0 : 1;

    } catch (const osdui::config::ParseError& e) {
        log.error(L"osdUI", std::wstring{e.what(), e.what() + strlen(e.what())});
        return 1;
    } catch (const std::exception& e) {
        log.error(L"osdUI", std::wstring{e.what(), e.what() + strlen(e.what())});
        return 1;
    }
}
```

- [ ] **Step 2: Build full solution**

```bash
cmake --build --preset windows-release
```

Expected: `build/windows-release/app/osdui.exe` produced.

- [ ] **Step 3: Commit**

```bash
git add app/main.cpp
git commit -m "feat: implement main.cpp entry point"
```

---

### Task 27: Final CI push and smoke test

- [ ] **Step 1: Push to GitHub and verify CI passes**

```bash
git push origin main
```

Expected: GitHub Actions workflow runs, builds both targets, all ctest tests pass, artifact `osdui-windows-release` uploaded.

- [ ] **Step 2: Deploy to a WinPE lab environment and run a basic config**

Test with a minimal `UI++.xml` containing `DefaultValues` + `Input` actions. Verify:
- Dialog appears
- Variable is written to TS environment
- osdui.exe exits 0
- Log file written in CMTrace format

- [ ] **Step 3: Commit any fixes found during lab testing**

---

## Notes for Implementers

- **Original code reference:** The original UI++ code is in `UI++/`, `FTWCMLog/`, `FTWldap/` etc. It remains in the repo as a reference. Do not modify it.
- **cpp standard:** All new code uses C++20. Use `std::format`, `std::ranges`, `std::span`, structured bindings freely.
- **No MFC:** Do not include `<afxwin.h>` or any MFC headers in core or app.
- **wil everywhere in app/:** Use `wil::unique_hkey`, `wil::com_ptr`, `wil::check_hresult` for all Win32 resource management.
- **Immutability:** Prefer `const` and immutable value types. Never mutate shared state without a lock.
- **One action per commit:** Each action type gets its own commit after its tests pass.
