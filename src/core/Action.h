#pragma once

enum class Action
{
    DeckA_Play,
    DeckA_Cue,
    DeckB_Play,
    DeckB_Cue,
    ChannelA_Gain,
    ChannelA_EqHigh,
    ChannelA_EqMid,
    ChannelA_EqLow,
    ChannelA_Filter,
    ChannelA_Fader,
    ChannelB_Gain,
    ChannelB_EqHigh,
    ChannelB_EqMid,
    ChannelB_EqLow,
    ChannelB_Filter,
    ChannelB_Fader,
    Mixer_Crossfader,
    Mixer_CrossfaderCurve
};

// Buttons: value 1 = pressed, 0 = released. Continuous controls: value 0–1.
struct ActionEvent
{
    Action action;
    float value;
};
