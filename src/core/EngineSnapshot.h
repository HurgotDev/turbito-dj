#pragma once

#include "core/DeckSnapshot.h"

// Written by the audio thread, read by the UI. Index 0 = deck A, 1 = deck B.
struct EngineSnapshot
{
    DeckSnapshot decks[2];
};
