add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(library)
add_subdirectory(ui)

juce_add_gui_app(TurbitoDJ
    PRODUCT_NAME "Turbito DJ"
    COMPANY_NAME "HurgotDev"
    BUNDLE_ID com.hurgotdev.turbitodj
    ICON_BIG ${PROJECT_SOURCE_DIR}/assets/brand/turbito-icon-1024.png
    ICON_SMALL ${PROJECT_SOURCE_DIR}/assets/brand/turbito-icon-256.png)

target_sources(TurbitoDJ PRIVATE
    app/Main.cpp
    app/MainComponent.cpp)

target_include_directories(TurbitoDJ PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(TurbitoDJ PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:TurbitoDJ,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:TurbitoDJ,JUCE_VERSION>")

target_link_libraries(TurbitoDJ PRIVATE turbito_engine turbito_library turbito_ui)
