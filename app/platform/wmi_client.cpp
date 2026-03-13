#include "wmi_client.hpp"
#include <windows.h>
#include <wbemidl.h>
#include <wil/com.h>
#include <wil/result.h>

namespace osdui::platform {

std::optional<std::wstring> WmiClient::query(std::wstring_view wql,
                                              std::wstring_view property) const {
    wil::com_ptr<IWbemLocator> locator;
    HRESULT hr = CoCreateInstance(CLSID_WbemLocator, nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IWbemLocator,
                                  reinterpret_cast<void**>(&locator));
    if (FAILED(hr)) return std::nullopt;

    wil::com_ptr<IWbemServices> services;
    wil::unique_bstr ns{SysAllocString(L"ROOT\\CIMV2")};
    hr = locator->ConnectServer(ns.get(), nullptr, nullptr, nullptr,
                                0, nullptr, nullptr, &services);
    if (FAILED(hr)) return std::nullopt;

    hr = CoSetProxyBlanket(services.get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                           nullptr, RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hr)) return std::nullopt;

    std::wstring wql_str{wql};
    wil::unique_bstr query_bstr{SysAllocString(wql_str.c_str())};
    wil::unique_bstr lang_bstr{SysAllocString(L"WQL")};
    wil::com_ptr<IEnumWbemClassObject> enumerator;
    hr = services->ExecQuery(lang_bstr.get(), query_bstr.get(),
                             WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                             nullptr, &enumerator);
    if (FAILED(hr)) return std::nullopt;

    wil::com_ptr<IWbemClassObject> obj;
    ULONG returned = 0;
    hr = enumerator->Next(WBEM_INFINITE, 1, &obj, &returned);
    if (FAILED(hr) || returned == 0) return std::nullopt;

    std::wstring prop{property};
    wil::unique_variant val;
    hr = obj->Get(prop.c_str(), 0, val.addressof(), nullptr, nullptr);
    if (FAILED(hr) || val.vt != VT_BSTR || val.bstrVal == nullptr)
        return std::nullopt;

    return std::wstring{val.bstrVal};
}

void WmiClient::set(std::wstring_view, std::wstring_view, std::wstring_view) {
    // WMI write not required for current use cases
}

} // namespace osdui::platform
