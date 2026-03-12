#include "action_random_string.hpp"
#include <random>
#include <string>

namespace osdui::actions {

namespace {

const wchar_t* charset_for(std::wstring_view cs) {
    if (cs == L"alpha")   return L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    if (cs == L"numeric") return L"0123456789";
    if (cs == L"all")     return L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
    // default: alphanumeric
    return L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
}

} // anon namespace

ActionResult RandomStringAction::execute(ActionContext& ctx) {
    const wchar_t* chars = charset_for(charset_);
    std::size_t char_count = std::wcslen(chars);

    std::random_device rd;
    std::mt19937 rng{rd()};
    std::uniform_int_distribution<std::size_t> dist{0, char_count - 1};

    std::wstring result;
    result.reserve(length_);
    for (int i = 0; i < length_; ++i)
        result += chars[dist(rng)];

    ctx.vars.set(variable_, result);
    return {};
}

} // namespace osdui::actions
