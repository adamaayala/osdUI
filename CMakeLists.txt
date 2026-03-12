cmake_minimum_required(VERSION 3.25)
project(osdUI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static CRT for WinPE compatibility
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(pugixml CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(WIL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(core)
add_subdirectory(app)

enable_testing()
add_subdirectory(tests)
