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
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:TurbitoDJ,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:TurbitoDJ,JUCE_VERSION>")

target_link_libraries(TurbitoDJ
    PRIVATE
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
