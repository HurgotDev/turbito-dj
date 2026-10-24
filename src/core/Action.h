#pragma once

enum class Action
{
    DeckA_Play,
    DeckA_Cue
};

// Buttons: value 1 = pressed, 0 = released.
struct ActionEvent
{
    Action action;
    float value;
};
