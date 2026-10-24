add_library(turbito_core INTERFACE)

target_include_directories(turbito_core INTERFACE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(turbito_core INTERFACE turbito_juce)
