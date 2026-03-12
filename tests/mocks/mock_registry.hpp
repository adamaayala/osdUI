#pragma once
#include <osdui/iregistry.hpp>
#include <map>
#include <tuple>

namespace osdui::test {

class MapRegistry : public IRegistry {
public:
    // Helper to pre-populate registry for tests
    void set(HKEY hive, std::wstring_view key, std::wstring_view value, std::wstring_view data) {
        store_[make_key(hive, key, value)] = std::wstring{data};
    }
    std::optional<std::wstring> read(HKEY hive, std::wstring_view key,
                                      std::wstring_view value) const override {
        auto it = store_.find(make_key(hive, key, value));
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }
    void write(HKEY hive, std::wstring_view key,
               std::wstring_view value, std::wstring_view data) override {
        store_[make_key(hive, key, value)] = std::wstring{data};
    }
private:
    static std::wstring make_key(HKEY hive, std::wstring_view key, std::wstring_view value) {
        return std::to_wstring(reinterpret_cast<uintptr_t>(hive))
               + L"\\" + std::wstring{key} + L"\\" + std::wstring{value};
    }
    std::map<std::wstring, std::wstring> store_;
};

} // namespace osdui::test
