#include "ts_variables.hpp"

#include <windows.h>
#include <oaidl.h>
#include <wil/com.h>
#include <wil/result.h>

#include <string>
#include <optional>

namespace osdui::platform {

TsVariables::TsVariables() {
    // COM must be initialized by the caller before constructing this object.

    // Create the SCCM task sequence environment COM object by ProgID.
    CLSID clsid{};
    THROW_IF_FAILED(CLSIDFromProgID(L"Microsoft.SMS.TSEnvironment", &clsid));

    wil::com_ptr<IDispatch> dispatch;
    // TSEnvironment is an out-of-process COM server hosted in TSManager.exe.
    // CLSCTX_ALL allows COM to reach the local server; CLSCTX_INPROC_SERVER
    // would fail with ERROR_MOD_NOT_FOUND because there is no in-proc DLL.
    HRESULT hr = CoCreateInstance(clsid, nullptr, CLSCTX_ALL,
                                  IID_IDispatch,
                                  reinterpret_cast<void**>(&dispatch));
    if (FAILED(hr)) {
        // Surface a clear message — the most common cause is running outside
        // an active SCCM task sequence (TSManager.exe not running).
        THROW_HR_MSG(hr,
            "Cannot connect to Microsoft.SMS.TSEnvironment. "
            "osdUI must run inside an active SCCM task sequence.");
    }
    ts_env_ = std::move(dispatch);
}

TsVariables::~TsVariables() = default;

std::optional<std::wstring> TsVariables::get(std::wstring_view name) const {
    // Build a mutable copy of the name as OLECHAR for GetIDsOfNames.
    std::wstring name_buf{name};
    OLECHAR* name_ptr = name_buf.data();

    DISPID dispid = DISPID_UNKNOWN;
    HRESULT hr = ts_env_->GetIDsOfNames(IID_NULL, &name_ptr, 1,
                                         LOCALE_USER_DEFAULT, &dispid);
    THROW_IF_FAILED(hr);

    // Build the property name as a BSTR argument for the indexed property get.
    wil::unique_bstr name_bstr{SysAllocString(name_buf.c_str())};
    THROW_HR_IF(E_OUTOFMEMORY, !name_bstr);

    VARIANT arg_var{};
    VariantInit(&arg_var);
    arg_var.vt   = VT_BSTR;
    arg_var.bstrVal = name_bstr.get();

    DISPPARAMS params{};
    params.cArgs            = 1;
    params.rgvarg           = &arg_var;
    params.cNamedArgs       = 0;
    params.rgdispidNamedArgs = nullptr;

    wil::unique_variant result;
    THROW_IF_FAILED(ts_env_->Invoke(dispid,
                                     IID_NULL,
                                     LOCALE_USER_DEFAULT,
                                     DISPATCH_PROPERTYGET,
                                     &params,
                                     result.addressof(),
                                     nullptr,
                                     nullptr));

    // The TS environment returns an empty string when a variable is not set.
    if (result.vt != VT_BSTR || result.bstrVal == nullptr ||
        SysStringLen(result.bstrVal) == 0) {
        return std::nullopt;
    }

    return std::wstring{result.bstrVal};
}

void TsVariables::set(std::wstring_view name, std::wstring_view value) {
    // Build a mutable copy of the name for GetIDsOfNames.
    std::wstring name_buf{name};
    OLECHAR* name_ptr = name_buf.data();

    DISPID dispid = DISPID_UNKNOWN;
    HRESULT hr = ts_env_->GetIDsOfNames(IID_NULL, &name_ptr, 1,
                                         LOCALE_USER_DEFAULT, &dispid);
    THROW_IF_FAILED(hr);

    // The TS environment uses an indexed property put:
    //   TSEnvironment(variableName) = value
    // We pass the variable name as the first arg and the value as the second.
    wil::unique_bstr name_bstr{SysAllocString(name_buf.c_str())};
    THROW_HR_IF(E_OUTOFMEMORY, !name_bstr);

    std::wstring value_buf{value};
    wil::unique_bstr value_bstr{SysAllocString(value_buf.c_str())};
    THROW_HR_IF(E_OUTOFMEMORY, !value_bstr);

    // For DISPATCH_PROPERTYPUT the first named arg must be DISPID_PROPERTYPUT.
    // Arguments are in reverse order: value first, then the index (name).
    VARIANT args[2]{};
    VariantInit(&args[0]);
    VariantInit(&args[1]);

    // args[0] = new value (rightmost in source, first in rgvarg reverse order)
    args[0].vt      = VT_BSTR;
    args[0].bstrVal = value_bstr.get();

    // args[1] = property index (variable name)
    args[1].vt      = VT_BSTR;
    args[1].bstrVal = name_bstr.get();

    DISPID named_arg = DISPID_PROPERTYPUT;
    DISPPARAMS params{};
    params.cArgs             = 2;
    params.rgvarg            = args;
    params.cNamedArgs        = 1;
    params.rgdispidNamedArgs = &named_arg;

    THROW_IF_FAILED(ts_env_->Invoke(dispid,
                                     IID_NULL,
                                     LOCALE_USER_DEFAULT,
                                     DISPATCH_PROPERTYPUT,
                                     &params,
                                     nullptr,
                                     nullptr,
                                     nullptr));
}

} // namespace osdui::platform
