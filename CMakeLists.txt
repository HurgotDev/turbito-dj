cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")

project(TurbitoDJ VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/Dependencies.cmake)
include(cmake/Sqlite.cmake)
include(cmake/QmDsp.cmake)
include(cmake/JuceShared.cmake)
include(cmake/Tokens.cmake)
include(cmake/BinaryData.cmake)

include(CTest)
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
