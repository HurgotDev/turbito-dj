include(FetchContent)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.4
    GIT_SHALLOW TRUE
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/juce-build)

FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.1
    GIT_SHALLOW TRUE
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/catch2-build)

# SQLite amalgamation (public domain): sources only, see cmake/Sqlite.cmake.
FetchContent_Declare(sqlite
    URL https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip
    URL_HASH SHA256=1d3049dd0f830a025a53105fc79fd2ab9431aea99e137809d064d8ee8356b032
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

# TagLib 2.x (LGPL-2.1 / MPL-1.1). Its utfcpp submodule is replaced by the
# utfcpp declaration below, so the submodule is not fetched.
FetchContent_Declare(TagLib
    GIT_REPOSITORY https://github.com/taglib/taglib.git
    GIT_TAG v2.3.2
    GIT_SHALLOW TRUE
    GIT_SUBMODULES ""
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/taglib-build)

# utfcpp (BSL-1.0, header-only): required by TagLib 2.x.
FetchContent_Declare(utfcpp
    GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
    GIT_TAG v4.2.1
    GIT_SHALLOW TRUE
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/utfcpp-build)

# qm-dsp (GPL-2.0-or-later, no CMake): pinned by commit, see cmake/QmDsp.cmake.
FetchContent_Declare(qmdsp
    GIT_REPOSITORY https://github.com/c4dm/qm-dsp.git
    GIT_TAG e34a3cc188332ed7c33cd9257ef164de5b587191
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/qmdsp-build)

FetchContent_MakeAvailable(JUCE)
FetchContent_MakeAvailable(Catch2)

cmake_policy(SET CMP0169 OLD)
# Sources only (no CMakeLists.txt): each cmake/*.cmake module adds its target.
FetchContent_Populate(sqlite)
FetchContent_Populate(utfcpp)
FetchContent_Populate(qmdsp)

# TagLib's options are set as normal variables, so they only apply to its
# directory and do not leak into the cache or into our BUILD_TESTING.
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_BINDINGS OFF)
set(WITH_ZLIB OFF)
# TagLib looks for utfcpp in this variable before any system install.
set(utf8cpp_INCLUDE_DIR ${utfcpp_SOURCE_DIR}/source)
FetchContent_MakeAvailable(TagLib)
unset(BUILD_SHARED_LIBS)
unset(BUILD_TESTING)
unset(BUILD_EXAMPLES)
unset(BUILD_BINDINGS)
unset(WITH_ZLIB)
unset(utf8cpp_INCLUDE_DIR)

# TagLib's in-tree target is `tag`; the namespaced name matches its install.
add_library(TagLib::tag ALIAS tag)

# In-tree the headers sit in TagLib's own subdirectories (toolkit, mpeg...),
# which its CMake only adds to its private include path. Public headers need
# them too, so take that list and expose it with the <taglib/...> root.
get_property(TAGLIB_HDR_DIRS DIRECTORY ${taglib_SOURCE_DIR}/taglib
    PROPERTY INCLUDE_DIRECTORIES)
target_include_directories(tag PUBLIC $<BUILD_INTERFACE:
    ${taglib_SOURCE_DIR};${taglib_SOURCE_DIR}/taglib;${taglib_BINARY_DIR};
    ${TAGLIB_HDR_DIRS}>)
