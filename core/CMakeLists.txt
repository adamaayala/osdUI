add_library(osdui-core STATIC)

target_include_directories(osdui-core
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(osdui-core
  PUBLIC  pugixml::pugixml
  PRIVATE CURL::libcurl
          nlohmann_json::nlohmann_json
)

# Glob sources — add each file explicitly as you create it
target_sources(osdui-core PRIVATE
  src/logging/cm_log.cpp
  src/config/config_parser.cpp
  src/script/condition_evaluator.cpp
  src/actions/action_runner.cpp
  src/actions/action_default_values.cpp
  src/actions/action_external_call.cpp
  src/actions/action_switch.cpp
  src/actions/action_random_string.cpp
  src/actions/action_reg_read.cpp
  src/actions/action_reg_write.cpp
)
