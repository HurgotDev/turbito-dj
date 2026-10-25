# SQLite amalgamation as a static C library. Sources come from
# FetchContent (cmake/Dependencies.cmake).
add_library(turbito_sqlite STATIC ${sqlite_SOURCE_DIR}/sqlite3.c)

target_include_directories(turbito_sqlite PUBLIC ${sqlite_SOURCE_DIR})

target_compile_definitions(turbito_sqlite PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_OMIT_LOAD_EXTENSION
    SQLITE_DQS=0)

# Third-party code: keep its warnings out of our build.
if(MSVC)
    target_compile_options(turbito_sqlite PRIVATE /W0)
else()
    target_compile_options(turbito_sqlite PRIVATE -w)
endif()
