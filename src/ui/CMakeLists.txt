add_library(turbito_ui INTERFACE)

target_include_directories(turbito_ui INTERFACE ${CMAKE_BINARY_DIR}/generated)

target_link_libraries(turbito_ui INTERFACE turbito_core turbito_assets)
