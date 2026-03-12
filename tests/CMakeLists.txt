find_package(Catch2 3 CONFIG REQUIRED)

add_executable(osdui-tests
  test_stub.cpp   # replaced by real tests in Chunk 2+
)

target_link_libraries(osdui-tests
  PRIVATE osdui-core
          Catch2::Catch2WithMain
)

target_include_directories(osdui-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(CTest)
list(APPEND CMAKE_MODULE_PATH "${Catch2_DIR}/../../../share/catch2")
include(Catch)
catch_discover_tests(osdui-tests)
