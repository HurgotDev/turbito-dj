add_library(turbito_library STATIC
    db/Database.cpp
    db/Schema.cpp
    db/TrackRepository.cpp
    scan/FolderScanner.cpp
    scan/TagReader.cpp)

# PUBLIC: later tasks (scan, analysis) and the dependency smoke test use their headers.
target_link_libraries(turbito_library PUBLIC turbito_core turbito_sqlite TagLib::tag turbito_qmdsp)
