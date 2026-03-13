# osdUI

A Windows C++20 application that provides a graphical user interface for Microsoft Configuration Manager (SCCM/ConfigMgr) task sequences during OS deployment.

Initial public open-source release: December 14, 2024.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Building](#building)
- [Running the Unit Tests](#running-the-unit-tests)
- [Deploying to a Task Sequence](#deploying-to-a-task-sequence)
- [Testing in a Deployment](#testing-in-a-deployment)
- [Reporting Bugs and Issues](#reporting-bugs-and-issues)

---

## Prerequisites

All of the following must be installed on a **Windows** machine. Building on macOS or Linux is not supported (the output is a Win32 executable).

| Requirement | Version | Notes |
|-------------|---------|-------|
| Windows 10/11 or Windows Server | Any current | Build machine only; the output runs on WinPE |
| Visual Studio 2026 | 18.x | Workload: **Desktop development with C++** |
| CMake | 3.25+ | Included with VS 2026, or install separately |
| vcpkg | Any current | See setup below |

### vcpkg Setup (one-time)

```powershell
# Clone vcpkg somewhere permanent
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat

# Set VCPKG_ROOT — add this to your system environment variables permanently
$env:VCPKG_ROOT = "C:\vcpkg"
[System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "Machine")
```

vcpkg dependencies (`pugixml`, `curl`, `wil`, `nlohmann-json`, `catch2`) are declared in `vcpkg.json` and installed automatically during the CMake configure step. No manual installation is required.

---

## Building

Open a **VS 2022 Developer Command Prompt** or a PowerShell session with the VS environment loaded, then run from the repo root:

```cmd
# Configure — downloads and builds all vcpkg dependencies on first run
cmake --preset windows-release

# Build
cmake --build --preset windows-release
```

Output: `build\windows-release\app\Release\osdui.exe`

### Debug Build

```cmd
cmake --preset windows-debug
cmake --build --preset windows-debug
```

Output: `build\windows-debug\app\Debug\osdui.exe`

### Opening in Visual Studio

After configuring, open Visual Studio and use **File > Open > CMake...** to open the root `CMakeLists.txt`. Visual Studio will detect the presets automatically.

---

## Running the Unit Tests

The unit tests run on the build machine — they do not require WinPE or a task sequence environment.

```cmd
ctest --preset windows-release --no-tests=error
```

For verbose output showing each test case:

```cmd
ctest --preset windows-release --no-tests=error -V
```

All 49 tests should pass. A failing test indicates a regression in the core logic layer.

---

## Deploying to a Task Sequence

### What You Need

- The built `osdui.exe`
- A config XML file (see `docs/` for schema reference)
- An SCCM/ConfigMgr task sequence

### Deployment Steps

1. **Copy `osdui.exe`** to a location accessible during the task sequence — typically a package source folder or the `_SMSTaskSequence` working directory.

2. **Create your config XML.** The XML must have the structure:
   ```xml
   <UIpp>
     <Actions>
       <Action Type="DefaultValues">...</Action>
       <Action Type="Input" Title="...">...</Action>
       <!-- etc. -->
     </Actions>
   </UIpp>
   ```

3. **Add a Run Command Line step** to your task sequence:
   ```
   osdui.exe /config:.\config.xml
   ```
   Optional: `/log:C:\Windows\Temp\osdui.log` to write a ConfigMgr-format log file.

4. **Place the step** before any steps that depend on variables collected by osdUI (typically near the start of the task sequence, after **Restart in Windows PE** if applicable).

5. **Set the step to run in the context** of the local system — osdUI reads and writes task sequence variables via the `Microsoft.SMS.TSEnvironment` COM object, which is only available when the task sequence engine is running.

### Required Task Sequence Environment

osdUI requires the SCCM task sequence engine (`TSManager.exe`) to be running. The `Microsoft.SMS.TSEnvironment` COM object must be registered and accessible. This is always the case during:

- A running OSD task sequence in WinPE
- A running OSD task sequence in the full OS (in-place upgrade, provisioning)

It is **not** available on a standard desktop session. For development testing, use a test VM with a running task sequence.

---

## Testing in a Deployment

### Smoke Test Checklist

Before rolling out to production, run through the following in a test deployment:

- [ ] osdUI launches without errors (check the log file)
- [ ] All expected dialogs appear in the correct order
- [ ] Variable values entered by the user are visible in later task sequence steps (use a **Set Task Sequence Variable** debug step to echo them)
- [ ] Preflight checks pass/fail/warn as expected
- [ ] Cancelling a dialog (if `AllowCancel="true"`) aborts the task sequence cleanly
- [ ] The task sequence continues normally after osdUI exits with code 0

### Checking Variables

Add a temporary **Run PowerShell Script** step immediately after the osdUI step to dump all collected variables:

```powershell
$tsenv = New-Object -COMObject Microsoft.SMS.TSEnvironment
$tsenv.GetVariables() | ForEach-Object { Write-Host "$_ = $($tsenv.Value($_))" }
```

Review the output in `smsts.log` to confirm variables were set correctly.

### Log File

osdUI writes a ConfigMgr-format log to the path specified by `/log:`. Open it with **CMTrace** or **OneTrace** for structured viewing. Key things to look for:

- `ParseError` — malformed config XML
- `Unknown error` — unhandled exception in the action runner
- Action names and execution order — confirms the config was parsed correctly

### Exit Codes

| Code | Meaning |
|------|---------|
| `0` | Success — task sequence should continue |
| `1` | Error — check the log; task sequence step should be set to fail on non-zero exit |

---

## Reporting Bugs and Issues

See [CONTRIBUTING.md](CONTRIBUTING.md) for full details on filing bugs, requesting features, and submitting code.

**Quick link:** [Open an issue](https://github.com/adamaayala/osdUI/issues/new/choose)
