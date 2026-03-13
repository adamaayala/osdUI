#include <windows.h>
#include <commctrl.h>
#include <wil/result.h>
#include <osdui/action_graph.hpp>
#include "../core/src/config/config_parser.hpp"
#include "../core/src/actions/action_runner.hpp"
#include "../core/src/script/condition_evaluator.hpp"
#include "../core/src/logging/cm_log.hpp"
#include "platform/ts_variables.hpp"
#include "platform/env_variables.hpp"
#include "platform/wsh_script_host.hpp"
#include "dialogs/dialog_presenter.hpp"

#include <optional>
#include <string>
#include <string_view>

// Helper: convert narrow string to wide using CP_ACP (error messages are ASCII)
static std::wstring widen(const char* s) {
    if (!s || *s == '\0') return {};
    int len = MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
    std::wstring result(static_cast<std::size_t>(len) - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s, -1, result.data(), len);
    return result;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int)
{
    // Initialize common controls (ListView, TreeView used in dialogs)
    INITCOMMONCONTROLSEX icc{sizeof(INITCOMMONCONTROLSEX),
                             ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES};
    THROW_HR_IF(E_FAIL, !InitCommonControlsEx(&icc));

    // Parse command line: /config:<path> /log:<path>
    // Handles quoted paths: /config:"C:\path with spaces\UI++.xml"
    // Uses word-boundary matching to avoid false matches inside quoted values.
    std::wstring config_path = L"UI++.xml";
    std::wstring log_path    = L"osdui.log";

    std::wstring cmdline{lpCmdLine};
    auto get_arg = [&](std::wstring_view prefix) -> std::optional<std::wstring> {
        size_t pos = 0;
        while (true) {
            pos = cmdline.find(prefix, pos);
            if (pos == std::wstring::npos) return std::nullopt;
            if (pos == 0 || cmdline[pos - 1] == L' ') break;  // word boundary
            pos += prefix.size();
        }
        pos += prefix.size();
        if (pos < cmdline.size() && cmdline[pos] == L'"') {
            ++pos;  // skip opening quote
            auto end = cmdline.find(L'"', pos);
            return cmdline.substr(pos, end == std::wstring::npos ? end : end - pos);
        }
        auto end = cmdline.find(L' ', pos);
        return cmdline.substr(pos, end == std::wstring::npos ? end : end - pos);
    };
    if (auto p = get_arg(L"/config:")) config_path = *p;
    if (auto p = get_arg(L"/log:"))    log_path    = *p;

    osdui::logging::CmLog log{log_path};
    log.info(L"osdUI", L"Starting osdUI");

    int exit_code = 1;
    try {
        // COM init inside the guarded region so failures produce a clean exit code.
        // scope_exit ensures CoUninitialize is called if init succeeds, even on exception.
        THROW_IF_FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
        auto com_cleanup = wil::scope_exit([] { CoUninitialize(); });

        osdui::config::ConfigParser parser;
        auto graph = parser.parse(config_path);

        // Try to connect to the SCCM task sequence environment.
        // Fall back to process environment variables when TSManager.exe is not
        // running (e.g. local testing on a full Windows machine).
        std::unique_ptr<IVariableStore> vars_ptr;
        try {
            vars_ptr = std::make_unique<osdui::platform::TsVariables>();
            log.info(L"osdUI", L"Connected to Microsoft.SMS.TSEnvironment");
        } catch (...) {
            log.info(L"osdUI",
                L"Microsoft.SMS.TSEnvironment unavailable — "
                L"falling back to process environment variables");
            vars_ptr = std::make_unique<osdui::platform::EnvVariables>();
        }
        IVariableStore& vars = *vars_ptr;

        osdui::platform::WshScriptHost    scripts;
        osdui::dialogs::DialogPresenter   dialogs{hInstance};
        osdui::script::ConditionEvaluator conditions;

        osdui::actions::ActionRunner runner;
        auto result = runner.run(graph, vars, dialogs, scripts, conditions, &log);

        const bool succeeded = (result == osdui::actions::RunResult::Success);
        log.info(L"osdUI", succeeded ? L"Completed successfully" : L"Aborted");
        exit_code = succeeded ? 0 : 1;

    } catch (const osdui::config::ParseError& e) {
        log.error(L"osdUI", L"Config parse error: " + widen(e.what()));
    } catch (const std::exception& e) {
        log.error(L"osdUI", widen(e.what()));
    } catch (...) {
        log.error(L"osdUI", L"Unknown error");
    }

    return exit_code;
}
