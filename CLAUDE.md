# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

osdUI is a Windows C++20 application that provides a graphical user interface for Microsoft Configuration Manager (SCCM/ConfigMgr) task sequences during OS deployment. It allows technicians and end users to interact with the deployment process via dialogs, input forms, and preflight checks.

## Build Requirements

- **Visual Studio 2022** (generator: `Visual Studio 17 2022`)
- **CMake 3.25+**
- **vcpkg** for dependency management (triplet: `x64-windows-static`)
- **Platform**: x64 only
- **Configurations**: Debug and Release

### Build Commands

```cmd
# Configure (from repo root)
cmake --preset windows-release

# Build
cmake --build --preset windows-release

# Run tests
ctest --preset windows-release --no-tests=error
```

Output binary: `build/windows-release/app/osdui.exe`

The CI pipeline (`.github/workflows/ci.yml`) configures, builds, and tests via CMake presets on `windows-latest`.

### Dependencies (via vcpkg)

- `pugixml` — XML parsing
- `curl` 7.83.1 (pinned) — HTTP client
- `wil` — Windows Implementation Library (RAII wrappers)
- `nlohmann-json` — JSON parsing for REST responses
- `catch2` — unit test framework

## Project Structure

```
osdUI/
├── CMakeLists.txt          # Root: find_package, add_subdirectory
├── CMakePresets.json       # Build presets (windows-release, windows-debug)
├── vcpkg.json              # Dependency manifest
├── core/                   # osdui-core static library (pure C++20, no Win32)
│   ├── CMakeLists.txt
│   ├── include/osdui/      # Public headers
│   │   ├── interfaces.hpp  # IVariableStore, IDialogPresenter, IScriptHost, IConditionEvaluator
│   │   ├── iregistry.hpp   # IRegistry
│   │   ├── iwmi.hpp        # IWmi
│   │   ├── ihttp_client.hpp# IHttpClient
│   │   ├── model.hpp       # DialogSpec, InputSpec, PreflightItem, SoftwareItem, etc.
│   │   └── action_graph.hpp# ActionGraph, ActionNode, ActionResult, ActionContext
│   └── src/
│       ├── actions/        # All IAction implementations
│       ├── config/         # ConfigParser (pugixml → ActionGraph)
│       ├── http/           # HttpClient (libcurl)
│       ├── logging/        # CmLog (ConfigMgr .log format)
│       └── script/         # ConditionEvaluator
├── app/                    # osdui.exe (Win32 thin layer)
│   ├── CMakeLists.txt
│   ├── main.cpp            # wWinMain: COM init, parse args, run ActionRunner
│   ├── platform/
│   │   ├── ts_variables.*  # IVariableStore via COM (SMS TSEnvironment)
│   │   └── wsh_script_host.* # IScriptHost via CreateProcessW / IActiveScript
│   ├── dialogs/            # IDialogPresenter + 9 Win32 dialog implementations
│   └── resources/          # osdui.rc, resource.h
└── tests/                  # Catch2 unit tests (cross-platform, no Win32)
    ├── CMakeLists.txt
    ├── fixtures/           # XML test fixtures
    └── mocks/              # Mock implementations of all interfaces
```

## Architecture

### Two-Layer Design

**`osdui-core`** (static lib) — pure C++20, no Win32 headers, cross-platform testable:
- All action business logic (`IAction` implementations)
- `ConfigParser`: XML → `ActionGraph`
- `ActionRunner`: executes `ActionGraph`, calls interfaces
- `ConditionEvaluator`: evaluates condition expressions
- `HttpClient`: libcurl wrapper
- `CmLog`: ConfigMgr log format writer

**`osdui.exe`** (Win32 thin layer):
- `TsVariables`: `IVariableStore` via COM `Microsoft.SMS.TSEnvironment`
- `WshScriptHost`: `IScriptHost` via `CreateProcessW` and `IActiveScript`
- `DialogPresenter`: `IDialogPresenter` dispatching to 9 Win32 dialog classes
- `wWinMain`: glues everything together, owns COM lifetime

### Key Interfaces (`core/include/osdui/interfaces.hpp`)

| Interface | Purpose |
|-----------|---------|
| `IVariableStore` | Get/set task sequence variables |
| `IDialogPresenter` | Show dialogs, return user input |
| `IScriptHost` | Execute scripts/processes |
| `IConditionEvaluator` | Evaluate condition expressions |
| `IRegistry` | Read/write Windows registry |
| `IWmi` | Execute WMI queries |
| `IHttpClient` | Make HTTP requests |

### Important Build Notes

- `NOMINMAX` and `WIN32_LEAN_AND_MEAN` are set globally to prevent `windows.h` macro conflicts with `std::numeric_limits`
- `UNICODE` and `_UNICODE` are set on the `app` target for wide Win32 APIs
- pugixml uses narrow `char*` by default — `config_parser.cpp` uses `to_wide()` (`MultiByteToWideChar`) to convert to `std::wstring`
- WIL CMake target is `WIL::WIL` (not `Microsoft::WIL`)
- COM is initialized in `main.cpp` (`COINIT_APARTMENTTHREADED`) — platform classes do NOT call `CoInitializeEx`
