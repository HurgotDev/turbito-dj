#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/mixer/MixerComponent.h"

namespace
{
    // Children are added channel A (5 knobs, VU, fader), channel B, the FX
    // unit, Placas and, last, the crossfader.
    std::vector<Knob*> knobs(MixerComponent& mixer)
    {
        std::vector<Knob*> result;

        for (auto* child : mixer.getChildren())
            if (auto* knob = dynamic_cast<Knob*>(child))
                result.push_back(knob);

        return result;
    }

    juce::Slider& crossfader(MixerComponent& mixer)
    {
        return dynamic_cast<juce::Slider&>(
            *mixer.getChildComponent(mixer.getNumChildComponents() - 1));
    }
}

TEST_CASE("Mixer controls send their channel actions", "[ui][mixer]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    MixerComponent mixer;
    std::vector<ActionEvent> events;
    mixer.onAction = [&](ActionEvent event) { events.push_back(event); };

    knobs(mixer)[5 + 3]->setValue(0.25, juce::sendNotificationSync);  // channel B, Graves
    crossfader(mixer).setValue(0.8, juce::sendNotificationSync);

    REQUIRE(events.size() == 2);
    CHECK(events[0].action == Action::ChannelB_EqLow);
    CHECK_THAT(events[0].value, Catch::Matchers::WithinAbs(0.25, 1e-6));
    CHECK(events[1].action == Action::Mixer_Crossfader);
    CHECK_THAT(events[1].value, Catch::Matchers::WithinAbs(0.8, 1e-6));
}

TEST_CASE("setControl moves a control without sending", "[ui][mixer]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    MixerComponent mixer;
    auto sent = 0;
    mixer.onAction = [&](ActionEvent) { ++sent; };

    mixer.setControl(Action::ChannelB_EqLow, 0.75f);
    mixer.setControl(Action::Mixer_Crossfader, 0.1f);

    CHECK(sent == 0);
    CHECK_THAT(knobs(mixer)[5 + 3]->getValue(), Catch::Matchers::WithinAbs(0.75, 1e-6));
    CHECK_THAT(crossfader(mixer).getValue(), Catch::Matchers::WithinAbs(0.1, 1e-6));
}

TEST_CASE("Mixer fits 300 × 400", "[ui][mixer]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    MixerComponent mixer;
    mixer.setBounds(0, 0, 300, MixerComponent::preferredHeight);

    for (auto* child : mixer.getChildren())
        CHECK(mixer.getLocalBounds().reduced(13).contains(child->getBounds()));
}
