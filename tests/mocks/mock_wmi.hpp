#pragma once
#include <osdui/iwmi.hpp>
#include <map>

namespace osdui::test {

class MapWmi : public IWmi {
public:
    // Helper to pre-populate for tests — keyed on wql+property
    void set_result(std::wstring_view wql, std::wstring_view property, std::wstring_view value) {
        store_[make_key(wql, property)] = std::wstring{value};
    }
    std::optional<std::wstring> query(std::wstring_view wql,
                                       std::wstring_view property) const override {
        auto it = store_.find(make_key(wql, property));
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }
    void set(std::wstring_view wql, std::wstring_view property,
             std::wstring_view value) override {
        store_[make_key(wql, property)] = std::wstring{value};
    }
private:
    static std::wstring make_key(std::wstring_view wql, std::wstring_view property) {
        return std::wstring{wql} + L"|" + std::wstring{property};
    }
    std::map<std::wstring, std::wstring> store_;
};

} // namespace osdui::test
