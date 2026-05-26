cmake_minimum_required(VERSION 3.15...4.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(
    PyLogMon
    DESCRIPTION "Log monitor python module"
    LANGUAGES CXX
    VERSION 1.0.0
)

# Find pybind11 and Python
find_package(pybind11 CONFIG REQUIRED)

# Add a library using FindPython's tooling (pybind11 also provides a helper like this)
python_add_library(_pylogmon MODULE src/module.cpp WITH_SOABI)
target_link_libraries(_pylogmon PRIVATE pybind11::headers)

# This is passing in the version as a define just as an example
target_compile_definitions(_pylogmon PRIVATE VERSION_INFO=${PROJECT_VERSION})

# The install directory is the output (wheel) directory
install(TARGETS _pylogmon DESTINATION pylogmon)
