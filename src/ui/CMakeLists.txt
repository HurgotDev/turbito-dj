add_library(turbito_ui STATIC
    look/Fonts.cpp
    look/TurbitoLookAndFeel.cpp
    widgets/Knob.cpp
    widgets/VuMeter.cpp
    widgets/JogWheel.cpp
    widgets/PadGrid.cpp
    widgets/WaveformView.cpp)

target_include_directories(turbito_ui PUBLIC ${CMAKE_BINARY_DIR}/generated)

target_link_libraries(turbito_ui PUBLIC turbito_core turbito_assets)
