cmake_minimum_required(VERSION 3.25)
project(osdUI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static CRT for WinPE compatibility
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Prevent windows.h from defining min/max macros (breaks std::numeric_limits)
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

find_package(pugixml CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(wil CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(core)
add_subdirectory(app)

enable_testing()
add_subdirectory(tests)
