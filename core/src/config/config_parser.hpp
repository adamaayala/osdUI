#pragma once
#include <osdui/action_graph.hpp>
#include <filesystem>
#include <stdexcept>

namespace osdui::config {

struct ParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ConfigParser {
public:
    // Parses the XML config file at path and returns an ActionGraph.
    // Throws ParseError on malformed XML or missing file.
    // Unknown action types are logged as warnings and skipped.
    ActionGraph parse(const std::filesystem::path& path) const;
};

} // namespace osdui::config
