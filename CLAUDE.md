# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

UI++ is a Windows C++/MFC application that provides a graphical user interface for Microsoft Configuration Manager (SCCM/ConfigMgr) task sequences during OS deployment. It allows technicians and end users to interact with the deployment process via dialogs, input forms, and preflight checks.

## Build Requirements

- **Visual Studio 2022** with Visual C++ and MFC static libraries
- **vcpkg** for dependency management (curl 7.83.1 pinned via `vcpkg.json`)
- **Platforms**: Win32 and x64
- **Configurations**: Debug and Release

### Build Commands

```cmd
# Integrate vcpkg
vcpkg integrate install

# Build (from VS Developer Command Prompt)
msbuild UI++.sln /p:Configuration=Release /p:Platform=x64
msbuild UI++.sln /p:Configuration=Release /p:Platform=Win32

# Debug build
msbuild UI++.sln /p:Configuration=Debug /p:Platform=x64
```

Output binaries land in `Build\Release\{Platform}\` (or `Build\Debug\{Platform}\`).

The CI pipeline (`.github/workflows/release.yml`) builds both platforms via MSBuild with vcpkg on `windows-latest`.

### curl Library Setup (if not using vcpkg)

Build curl 7.83.1 from source four times (x86/x64 × Debug/Release):
```
nmake /f Makefile.vc VC=14 mode=static ENABLE_SSPI=yes ENABLE_IPV6=yes ENABLE_SCHANNEL=yes ENABLE_UNICODE=yes machine=x64 DEBUG=yes
```
Then update Additional Library Directories in the UI++ project properties.

## Solution Structure

| Project | Output | Purpose |
|---------|--------|---------|
| `UI++/` | `UI++.exe` | Main MFC application |
| `FTWCMLog/` | `FTWCMLog.dll` | ConfigMgr/SCCM logging DLL |
| `FTWldap/` | `FTWldap.dll` | LDAP query DLL |
| `UI++AWS/` | DLL | AWS integration extension |
| `UI++REST/` | DLL | REST API integration extension |

## Architecture: UI++ Main Project

### Key Subsystems

**Actions** (`UI++/Actions/`)
- `IAction` — base interface for all actions
- `Actions.cpp/h` — action registry/factory
- `ActionUserInput` — drives the UserInput dialog
- `ActionDefaultValues` — sets default task sequence variable values
- `ActionExternalCall` — executes external programs/scripts
- `InteractiveActions.cpp` — actions that show UI to the user

**Dialogs** (`UI++/Dialogs/`)
- `DlgBase` — common MFC dialog base class
- `DlgUserInfo` / `DlgUserInfoFullScreen` — main user input form
- `DlgUserInput` — generic input dialog
- `DlgPreflight` — preflight check display
- `DlgProgress` — progress indicator
- `DlgAppTree` — software/application selection tree
- `DlgTSVar` — task sequence variable viewer
- `DlgUserAuth` — user authentication dialog

**Core Utilities** (`UI++/FTW/`)
- `CMLog` — logging to ConfigMgr log format
- `FTWHTTP` — HTTP client (wraps curl)
- `FTWControl` / `FTWBrowseEdit` / `FTWTreeCtrl` — custom MFC controls
- `TSProgress` — task sequence progress integration
- `Utils.cpp` — general utilities

**Top-level**
- `TSVar.cpp/h` — task sequence variable read/write
- `Software.cpp/h` — software catalog integration
- `ScriptHost.cpp/h` — Windows Script Host execution
- `Constants.h` — application-wide constants

### Third-Party Code
- `UI++/CodeProject/` — CodeProject.com sourced components
- `UI++/HansDietrich/` — HansDietrich MFC extensions
- `UI++/RCa10892/` — additional third-party MFC code

## Documentation Site

`Docs/website/` is a Jekyll site (TeXt theme). Run locally with:
```bash
cd Docs/website
bundle install
bundle exec jekyll serve
```
