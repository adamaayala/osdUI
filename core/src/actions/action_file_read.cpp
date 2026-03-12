#include "action_file_read.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace osdui::actions {

ActionResult FileReadAction::execute(ActionContext& ctx) {
    if (path_.empty()) return {};

    std::error_code ec;
    if (!std::filesystem::exists(path_, ec) || ec) return {};

    std::ifstream f{path_};
    if (!f) return {};

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string bytes = ss.str();

    // Convert UTF-8 to wstring
    std::wstring content(bytes.begin(), bytes.end());
    if (!variable_.empty())
        ctx.vars.set(variable_, content);

    return {};
}

} // namespace osdui::actions
