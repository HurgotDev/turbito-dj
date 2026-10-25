add_library(turbito_library STATIC
    analysis/AnalysisQueue.cpp
    analysis/BeatAnalyzer.cpp
    analysis/Camelot.cpp
    analysis/KeyAnalyzer.cpp
    analysis/TrackAnalyzer.cpp
    analysis/WaveformBuilder.cpp
    Library.cpp
    db/Database.cpp
    db/Schema.cpp
    db/TrackRepository.cpp
    scan/FolderScanner.cpp
    scan/TagReader.cpp)

# PUBLIC: later tasks (scan, analysis) and the dependency smoke test use their headers.
target_link_libraries(turbito_library PUBLIC turbito_core turbito_sqlite TagLib::tag turbito_qmdsp)
