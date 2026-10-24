include(Catch)

add_executable(turbito_tests
    ui/look/TokensTests.cpp)

target_include_directories(turbito_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_compile_definitions(turbito_tests PRIVATE
    TURBITO_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

target_link_libraries(turbito_tests
    PRIVATE
        Catch2::Catch2WithMain
        turbito_core)

catch_discover_tests(turbito_tests)
