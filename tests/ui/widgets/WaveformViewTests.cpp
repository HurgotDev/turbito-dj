#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/widgets/PaintHelpers.h"
#include "ui/widgets/WaveformView.h"

TEST_CASE("WaveformView draws the playhead at the centre", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    WaveformView waveform;
    waveform.setLoop(juce::Range<float>(0.5f, 0.59f));

    const auto image = paintWidget(waveform, 200, tokens::layoutWaveformHeight);

    CHECK(image.getPixelAt(100, 27).getARGB() == tokens::playhead);
}
