#pragma once

#include <functional>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

// "Audio: 44.1 kHz · buffer 256"; "Audio: sin dispositivo" when the sample
// rate is 0.
juce::String formatAudioStatus(double sampleRate, int bufferSize);

// Top bar: logo, controller and audio status, master BPM, Grabar and Ajustes.
class TopBar : public juce::Component
{
public:
    static constexpr int preferredHeight = 52;

    TopBar();

    void setController(bool connected, const juce::String& name);
    void setAudio(double sampleRate, int bufferSize);
    void setMasterBpm(const juce::String& bpm);

    std::function<void()> onSettings;
    std::function<void()> onRecord;  // nothing records until phase 13

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    // A LookAndFeel button background with custom content on top.
    struct PaintedButton : juce::Button
    {
        explicit PaintedButton(const juce::String& name);
        void paintButton(juce::Graphics&, bool isHighlighted, bool isDown) override;
        std::function<void(juce::Graphics&)> paintContent;
    };

    std::unique_ptr<juce::Drawable> logo;
    bool controllerConnected = false;
    juce::String controller, audio, masterBpm;

    PaintedButton record, settings;

    juce::Rectangle<int> logoArea, controllerArea, audioArea, masterArea;
};
