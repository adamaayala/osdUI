#include "wsh_script_host.hpp"

#include <windows.h>
#include <activscp.h>
#include <wil/com.h>
#include <wil/resource.h>
#include <wil/result.h>

namespace osdui::platform {

// ---------------------------------------------------------------------------
// IActiveScriptSite minimal implementation
// ---------------------------------------------------------------------------

class ScriptSite : public IActiveScriptSite {
public:
    ScriptSite() : ref_{1} {}

    // IUnknown
    ULONG __stdcall AddRef() override {
        return static_cast<ULONG>(InterlockedIncrement(&ref_));
    }

    ULONG __stdcall Release() override {
        LONG ref = InterlockedDecrement(&ref_);
        if (ref == 0) {
            delete this;
        }
        return static_cast<ULONG>(ref);
    }

    HRESULT __stdcall QueryInterface(REFIID iid, void** obj) override {
        if (!obj) return E_POINTER;
        *obj = nullptr;
        if (IsEqualIID(iid, IID_IUnknown)) {
            *obj = static_cast<IUnknown*>(this);
        } else if (IsEqualIID(iid, IID_IActiveScriptSite)) {
            *obj = static_cast<IActiveScriptSite*>(this);
        } else {
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    // IActiveScriptSite — minimal stubs
    HRESULT __stdcall GetLCID(LCID*) override { return E_NOTIMPL; }
    HRESULT __stdcall GetItemInfo(LPCOLESTR, DWORD, IUnknown**, ITypeInfo**) override {
        return E_NOTIMPL;
    }
    HRESULT __stdcall GetDocVersionString(BSTR*) override { return E_NOTIMPL; }
    HRESULT __stdcall OnScriptTerminate(const VARIANT*, const EXCEPINFO*) override {
        return S_OK;
    }
    HRESULT __stdcall OnStateChange(SCRIPTSTATE) override { return S_OK; }
    HRESULT __stdcall OnEnterScript() override { return S_OK; }
    HRESULT __stdcall OnLeaveScript() override { return S_OK; }
    HRESULT __stdcall OnScriptError(IActiveScriptError*) override { return S_OK; }

private:
    LONG ref_;
};

// ---------------------------------------------------------------------------
// Path A: CreateProcess execution
// ---------------------------------------------------------------------------

static model::ScriptResult run_exe(std::wstring_view command_line) {
    // CreateProcessW requires a mutable command line buffer.
    std::wstring cmd_buf{command_line};

    STARTUPINFOW si{};
    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};

    BOOL created = CreateProcessW(
        nullptr,
        cmd_buf.data(),
        nullptr,       // process security attrs
        nullptr,       // thread security attrs
        FALSE,         // inherit handles
        0,             // creation flags
        nullptr,       // environment (inherit)
        nullptr,       // current directory (inherit)
        &si,
        &pi);

    THROW_LAST_ERROR_IF(!created);

    // Take ownership of the handles via wil RAII.
    wil::unique_handle process_handle{pi.hProcess};
    wil::unique_handle thread_handle{pi.hThread};

    // Fix 1: Check WaitForSingleObject return value.
    const DWORD wait_result = WaitForSingleObject(process_handle.get(), INFINITE);
    THROW_HR_IF(HRESULT_FROM_WIN32(GetLastError()), wait_result == WAIT_FAILED);

    DWORD exit_code = 0;
    THROW_IF_WIN32_BOOL_FALSE(
        GetExitCodeProcess(process_handle.get(), &exit_code));

    return model::ScriptResult{static_cast<int>(exit_code), L""};
}

// ---------------------------------------------------------------------------
// Path B: IActiveScript / IActiveScriptParse execution
// ---------------------------------------------------------------------------

static model::ScriptResult run_script(std::wstring_view language,
                                      std::wstring_view source) {
    // Resolve CLSID from the language ProgID (e.g. "VBScript", "JScript").
    std::wstring lang_buf{language};
    CLSID clsid{};
    THROW_IF_FAILED(CLSIDFromProgID(lang_buf.c_str(), &clsid));

    wil::com_ptr<IActiveScript> active_script;
    THROW_IF_FAILED(CoCreateInstance(clsid,
                                     nullptr,
                                     CLSCTX_INPROC_SERVER,
                                     IID_IActiveScript,
                                     reinterpret_cast<void**>(&active_script)));

    wil::com_ptr<IActiveScriptParse> active_script_parse;
    THROW_IF_FAILED(active_script->QueryInterface(
        IID_IActiveScriptParse,
        reinterpret_cast<void**>(&active_script_parse)));

    // ScriptSite ref count starts at 1; IActiveScript::SetScriptSite AddRefs.
    // Wrap in a com_ptr so it is released when we exit scope.
    wil::com_ptr<IActiveScriptSite> site{new ScriptSite};
    THROW_IF_FAILED(active_script->SetScriptSite(site.get()));

    THROW_IF_FAILED(active_script_parse->InitNew());

    // Fix 3: Transition from SCRIPTSTATE_INITIALIZED to SCRIPTSTATE_STARTED
    // so that ParseScriptText can actually execute code.
    THROW_IF_FAILED(active_script->SetScriptState(SCRIPTSTATE_STARTED));

    // Fix 2: Use flag 0 (not SCRIPTTEXT_ISEXPRESSION) so statement-block
    // scripts (the common case for VBScript/JScript payloads) execute
    // correctly.
    // Fix 4: Capture EXCEPINFO to surface parse/execution error descriptions.
    std::wstring source_buf{source};
    EXCEPINFO ei{};
    const HRESULT hr = active_script_parse->ParseScriptText(
        source_buf.c_str(),
        nullptr,   // item name
        nullptr,   // context
        nullptr,   // delimiter
        0,         // source context cookie
        0,         // start line number
        SCRIPTTEXT_ISPERSISTENT,
        nullptr,
        &ei);

    if (FAILED(hr)) {
        std::wstring err = (ei.bstrDescription && *ei.bstrDescription)
                           ? std::wstring{ei.bstrDescription}
                           : L"Script parse/execution error";
        SysFreeString(ei.bstrSource);
        SysFreeString(ei.bstrDescription);
        SysFreeString(ei.bstrHelpFile);
        return model::ScriptResult{static_cast<int>(hr), err};
    }

    SysFreeString(ei.bstrSource);
    SysFreeString(ei.bstrDescription);
    SysFreeString(ei.bstrHelpFile);

    return model::ScriptResult{0, L""};
}

// ---------------------------------------------------------------------------
// IScriptHost::execute
// ---------------------------------------------------------------------------

model::ScriptResult WshScriptHost::execute(std::wstring_view language,
                                            std::wstring_view script) {
    if (language.empty() || language == L"exe") {
        return run_exe(script);
    }
    return run_script(language, script);
}

} // namespace osdui::platform
