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
