add_library(turbito_engine STATIC
    AudioEngine.cpp
    deck/Deck.cpp
    deck/TrackLoader.cpp
    mixer/Channel.cpp)

target_link_libraries(turbito_engine PUBLIC turbito_core)
