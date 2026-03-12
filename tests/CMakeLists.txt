find_package(Catch2 3 CONFIG REQUIRED)

add_executable(osdui-tests
  test_stub.cpp   # replaced by real tests in Chunk 2+
  test_interfaces_compile.cpp
  test_config_parser.cpp
  test_condition_evaluator.cpp
  test_action_runner.cpp
  test_action_default_values.cpp
  test_action_external_call.cpp
  test_action_switch.cpp
  test_action_random_string.cpp
  test_action_reg.cpp
  test_action_wmi.cpp
  test_action_file_read.cpp
  test_action_batch.cpp
  test_action_rest.cpp
)

target_link_libraries(osdui-tests
  PRIVATE osdui-core
          Catch2::Catch2WithMain
)

target_include_directories(osdui-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(osdui-tests PRIVATE
  FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

include(CTest)
list(APPEND CMAKE_MODULE_PATH "${Catch2_DIR}/../../../share/catch2")
include(Catch)
catch_discover_tests(osdui-tests)
