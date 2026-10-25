include(Catch)

add_executable(turbito_tests
    core/SpscQueueTests.cpp
    engine/AudioEngineTests.cpp
    engine/deck/DeckTests.cpp
    engine/deck/TrackLoaderTests.cpp
    engine/mixer/ChannelTests.cpp
    engine/mixer/CrossfaderTests.cpp
    library/DependenciesTests.cpp
    library/QueryTests.cpp
    library/analysis/AnalysisQueueTests.cpp
    library/analysis/CamelotTests.cpp
    library/analysis/TrackAnalyzerTests.cpp
    library/analysis/WaveformBuilderTests.cpp
    library/db/DatabaseTests.cpp
    library/db/SchemaTests.cpp
    library/db/TrackRepositoryTests.cpp
    library/scan/FolderScannerTests.cpp
    library/scan/TagReaderTests.cpp
    ui/AppViewSnapshotTests.cpp
    ui/AppViewTests.cpp
    ui/deck/DeckComponentTests.cpp
    ui/deck/WaveformPanelTests.cpp
    ui/library/AutoMixPanelTests.cpp
    ui/library/LibraryComponentTests.cpp
    ui/library/PanelResizerTests.cpp
    ui/look/FontsTests.cpp
    ui/look/TokensTests.cpp
    ui/look/TurbitoLookAndFeelTests.cpp
    ui/mixer/MixerComponentTests.cpp
    ui/topbar/TopBarTests.cpp
    ui/widgets/JogWheelTests.cpp
    ui/widgets/KnobTests.cpp
    ui/widgets/PadGridTests.cpp
    ui/widgets/VuMeterTests.cpp
    ui/widgets/WaveformViewTests.cpp)

target_include_directories(turbito_tests PRIVATE ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(turbito_tests PRIVATE
    TURBITO_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    TURBITO_BINARY_DIR="${PROJECT_BINARY_DIR}")

target_link_libraries(turbito_tests
    PRIVATE
        Catch2::Catch2WithMain
        turbito_engine
        turbito_library
        turbito_ui)

catch_discover_tests(turbito_tests)
