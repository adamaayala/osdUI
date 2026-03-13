# Contributing to osdUI

Thank you for taking the time to report a bug, request a feature, or submit a fix.

---

## Table of Contents

- [Reporting Bugs](#reporting-bugs)
- [Requesting Features](#requesting-features)
- [Submitting Code](#submitting-code)
- [Development Setup](#development-setup)

---

## Reporting Bugs

Use the [GitHub Issues](https://github.com/adamaayala/osdUI/issues) tracker. Before opening a new issue, search existing issues to avoid duplicates.

### What Makes a Good Bug Report

A good bug report lets someone reproduce the problem without asking follow-up questions. Include all of the following:

#### 1. Environment

| Field | Example |
|-------|---------|
| osdUI version / commit | `561b9eb` or `v1.0.0` |
| SCCM / ConfigMgr version | `2309 (5.00.9106)` |
| WinPE version | `WinPE 10 (ADK 10.1.26100)` |
| Target hardware make/model | `HP EliteBook 840 G10` |
| Task sequence type | `OSD`, `In-place upgrade`, `Provisioning` |

#### 2. Config XML

Paste the relevant portion of your config XML (or the whole file if it's short). Redact any sensitive values but keep the structure intact.

```xml
<UIpp>
  <Actions>
    <Action Type="Input" Title="User Details">
      <InputField Type="Text" Variable="OSDUserName" Prompt="Name" Required="true"/>
    </Action>
  </Actions>
</UIpp>
```

#### 3. What Happened

Describe what osdUI did, including:
- Which dialog appeared (or didn't appear)
- Any error message shown on screen
- The exit code of the osdUI step in the task sequence

#### 4. What You Expected

Describe what should have happened instead.

#### 5. Log File

Attach the osdUI log file. Collect it by adding `/log:C:\Windows\Temp\osdui.log` to the osdUI command line, then retrieve it from that path after the failure.

If the log is long, paste the relevant section — look for lines containing `error`, `Error`, or `ParseError`.

Open the log with **CMTrace** or **OneTrace** for easier reading before copying.

#### 6. smsts.log Excerpt (if applicable)

If the task sequence itself behaved unexpectedly after osdUI exited, include the relevant section of `smsts.log` (found at `C:\Windows\CCM\Logs\SMSTSLog\smsts.log` or `X:\Windows\Temp\SMSTSLog\smsts.log` in WinPE).

### Bug Report Template

When you open a new issue, use this template:

```
**Environment**
- osdUI version/commit:
- ConfigMgr version:
- WinPE version:
- Hardware:
- TS type:

**Config XML**
<paste here>

**What happened**
<describe the behavior>

**What I expected**
<describe the expected behavior>

**osdUI log**
<paste relevant lines or attach file>

**smsts.log excerpt** (if applicable)
<paste relevant lines>
```

---

## Requesting Features

Open a [GitHub Issue](https://github.com/adamaayala/osdUI/issues/new) with the label `enhancement`.

Include:
- **What you want** — describe the action type, dialog behavior, or variable handling you need
- **Why you need it** — the deployment scenario that requires it
- **Example config XML** — show how you'd expect to configure it if it existed

---

## Submitting Code

### Before You Start

- Open an issue first for significant changes so the approach can be discussed before you invest time writing code
- For small bug fixes, a PR without a prior issue is fine

### Workflow

1. Fork the repository
2. Create a branch: `git checkout -b fix/description` or `feat/description`
3. Make your changes following the structure below
4. Run the tests: `ctest --preset windows-release --no-tests=error`
5. Open a pull request against `main`

### Where Code Lives

| What | Where |
|------|-------|
| Action business logic | `core/src/actions/` |
| XML parsing | `core/src/config/config_parser.cpp` |
| Data model / interfaces | `core/include/osdui/` |
| Win32 dialogs | `app/dialogs/` |
| Platform implementations (COM, scripting) | `app/platform/` |
| Unit tests | `tests/` |
| Test mocks | `tests/mocks/` |

### Code Rules

- New actions go in `core/src/actions/` and must not include any Win32 headers — the core library is cross-platform testable
- New action types must be registered in `core/src/config/config_parser.cpp`
- Every new action needs a corresponding unit test in `tests/`
- Use the existing mock interfaces in `tests/mocks/` rather than adding Win32 dependencies to tests
- Follow the existing naming conventions: `action_snake_case.hpp/.cpp`, `dialog_snake_case.hpp/.cpp`

---

## Development Setup

See [README.md](README.md) for full build instructions. The short version:

```cmd
# One-time: set VCPKG_ROOT to your vcpkg installation
set VCPKG_ROOT=C:\vcpkg

# Configure and build
cmake --preset windows-release
cmake --build --preset windows-release

# Test
ctest --preset windows-release --no-tests=error
```
