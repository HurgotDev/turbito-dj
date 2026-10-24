# JUCE "shared code": the modules are compiled once, into turbito_juce. Other
# targets only receive their headers and definitions, so no JUCE object is
# duplicated across the static libraries and the app.
add_library(turbito_juce STATIC)

target_link_libraries(turbito_juce
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(turbito_juce PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_STANDALONE_APPLICATION=1)

foreach(module IN ITEMS
        juce_core juce_audio_basics juce_audio_formats juce_audio_devices
        juce_audio_utils juce_gui_basics)
    target_include_directories(turbito_juce INTERFACE
        $<TARGET_PROPERTY:${module},INTERFACE_INCLUDE_DIRECTORIES>)
    target_compile_definitions(turbito_juce INTERFACE
        $<TARGET_PROPERTY:${module},INTERFACE_COMPILE_DEFINITIONS>)
endforeach()
