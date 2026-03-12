#include <catch2/catch_test_macros.hpp>
#include <osdui/action_graph.hpp>
#include <filesystem>
// ConfigParser not yet included — this will fail to compile first:
#include "../../core/src/config/config_parser.hpp"

// FIXTURES_DIR is defined by CMake as the absolute path to tests/fixtures/
static const std::filesystem::path fixtures{FIXTURES_DIR};

TEST_CASE("ConfigParser parses basic XML") {
    osdui::config::ConfigParser parser;
    auto graph = parser.parse(fixtures / "basic.xml");
    REQUIRE(graph.nodes.size() == 2);
}

TEST_CASE("ConfigParser: missing file throws") {
    osdui::config::ConfigParser parser;
    REQUIRE_THROWS(parser.parse(fixtures / "nonexistent.xml"));
}
