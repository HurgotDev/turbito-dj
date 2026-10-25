#pragma once

#include <cmath>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/AppView.h"
#include "ui/look/Tokens.h"

// The sample state of design/mockup-reference.html's script, so a render of
// AppView can be compared with design/screenshots/mockup-1440x900.png.
namespace mockup
{
    inline juce::String utf8(const char* text) { return juce::String::fromUTF8(text); }

    // The mockup's wave(): 160 bars from a seed; emptyBefore leaves the first
    // half before the track starts.
    inline std::vector<float> wave(double seed, bool emptyBefore)
    {
        constexpr int count = 160;
        std::vector<float> bars;

        for (int i = 0; i < count; ++i)
        {
            if (emptyBefore && i < count / 2)
            {
                bars.push_back(0.0f);
                continue;
            }

            const auto env = 0.35 + 0.65 * std::abs(std::sin(i * 0.045 + seed));
            const auto tex = 0.45 + 0.55 * std::abs(std::sin(i * 0.9 + seed * 3) * std::cos(i * 0.31 + seed));
            bars.push_back(static_cast<float>(juce::jmax(4.0, std::round(48.0 * env * tex))));
        }

        return bars;
    }

    // The mockup's knob rotation v · 135° as a 0–1 value.
    inline float knob(float v) { return (v + 1.0f) / 2.0f; }

    inline juce::File trackFile(const juce::String& title) { return juce::File("/music").getChildFile(title + ".mp3"); }

    inline void apply(AppView& view)
    {
        auto& top = view.topBar();
        top.setController(true, "Numark Party Mix conectada");
        top.setAudio(44100.0, 256);
        top.setMasterBpm("124.0");

        // Deck A: playing at 2:41 of 6:12 (the formula can't reach the mockup's 118°; 161 s is 160°).
        auto& a = view.deck(0);
        a.setTrack(utf8("Luces de Neón"), "Varela & Sonido Norte");
        a.setBpm("124.0");
        a.setKey("8A");
        a.setPlaying(true);
        a.setPosition(161.0, 372.0);
        a.setPitch(0.0f);

        const char* cueLabels[] = { "Intro", "Verso", "Build", "Drop", "Break", "Drop 2", "Outro" };
        auto cues = PadGrid::defaultPads(PadGrid::Mode::HotCue);
        for (std::size_t i = 0; i < tokens::hotCues.size(); ++i)
            cues[i] = { cueLabels[i], juce::Colour(tokens::hotCues[i]), juce::Colour(tokens::bg) };
        a.pads().setPads(PadGrid::Mode::HotCue, cues);
        a.pads().setMode(PadGrid::Mode::HotCue);

        // Deck B: 126 BPM at −1.6 %, stopped at the cue point, SYNC on.
        auto& b = view.deck(1);
        b.setTrack("Medianoche en Polanco", "Kiro Beat");
        b.setBpm("124.0");
        b.setKey("5A");
        b.setPlaying(false);
        b.setPosition(0.0, 348.0);
        b.setPitch(-1.6f);
        b.setSync(true);

        const char* placaLabels[] = { "Saludo DJ", "Air horn", "Sirena", "Arriba las manos",
                                      "Placa evento", "Rewind", "Explosión" };
        auto placas = PadGrid::defaultPads(PadGrid::Mode::Placas);
        for (std::size_t i = 0; i < 7; ++i)
            placas[i] = { utf8(placaLabels[i]), juce::Colour(tokens::placas[i % 2]), juce::Colour(tokens::bg) };
        b.pads().setPads(PadGrid::Mode::Placas, placas);
        b.pads().setMode(PadGrid::Mode::Placas);

        view.waveforms().waveform(0).setBars(wave(1.3, false));
        view.waveforms().waveform(0).setLoop(juce::Range<float>(0.50f, 0.59f));
        view.waveforms().waveform(1).setBars(wave(2.7, true));

        auto& mixer = view.mixer();
        const std::pair<Action, float> controls[] = {
            { Action::ChannelA_Gain, knob(0.0f) },   { Action::ChannelA_EqHigh, knob(0.1f) },
            { Action::ChannelA_EqMid, knob(-0.2f) }, { Action::ChannelA_EqLow, knob(-0.6f) },
            { Action::ChannelA_Filter, knob(0.0f) }, { Action::ChannelA_Fader, 0.85f },
            { Action::ChannelB_Gain, knob(0.1f) },   { Action::ChannelB_EqHigh, knob(0.0f) },
            { Action::ChannelB_EqMid, knob(0.0f) },  { Action::ChannelB_EqLow, knob(0.0f) },
            { Action::ChannelB_Filter, knob(0.0f) }, { Action::ChannelB_Fader, 0.70f },
            { Action::Mixer_Crossfader, 0.5f },
        };
        for (const auto& [action, value] : controls)
            mixer.setControl(action, value);

        // round(11 · 0.85) = 9 lit segments: segment 8 lights at −15 dBFS, 9 at −12.
        mixer.setPeak(0, juce::Decibels::decibelsToGain(-13.5f));
        mixer.setPeak(1, 0.0f);
        mixer.setPlacasVolume(0.7f);

        auto& library = view.library();
        library.setCollections({ { "Todas las pistas", "1,284" },
                                 { "Carpetas", "12" },
                                 { utf8("Playlist · Boda sábado"), "48" },
                                 { utf8("Playlist · Warm-up"), "22" },
                                 { "Historial", "" },
                                 { "Placas y drops", "36" } });
        library.setSelectedCollection(0);

        const char* raw[][5] = { { "Luces de Neón", "Varela & Sonido Norte", "124", "8A", "6:12" },
                                 { "Medianoche en Polanco", "Kiro Beat", "126", "5A", "5:48" },
                                 { "Fuego Lento", "La Brava", "122", "11B", "4:55" },
                                 { "Pulso", "Dani Ortega", "128", "2A", "6:30" },
                                 { "Calor de Verano", "Nativa Club", "124", "9B", "3:58" },
                                 { "Tierra Mojada", "Ramiro Sosa", "120", "4A", "7:05" },
                                 { "Ritmo Callejero", "Los Bajos", "126", "7A", "5:21" } };
        std::vector<LibraryTrack> tracks;
        for (const auto& r : raw)
            tracks.push_back({ trackFile(utf8(r[0])), utf8(r[0]), r[1], r[2], r[3], r[4] });
        library.setTracks(tracks);
        library.setLoadedDeck(0, tracks[0].file);
        library.setLoadedDeck(1, tracks[1].file);

        using S = QueueStatus;
        view.autoMix().setQueue({ { "Calor de Verano", "Nativa Club", "124", "9B", "3:58", S::Played },
                                  { utf8("Luces de Neón"), "Varela & Sonido Norte", "124", "8A", "6:12", S::Playing },
                                  { "Medianoche en Polanco", "Kiro Beat", "126", "5A", "5:48", S::Next },
                                  { "Fuego Lento", "La Brava", "122", "11B", "4:55", S::Queued },
                                  { "Pulso", "Dani Ortega", "128", "2A", "6:30", S::Queued },
                                  { "Calor de Verano", "Nativa Club", "124", "9B", "3:58", S::Queued },
                                  { "Ritmo Callejero", "Los Bajos", "126", "7A", "5:21", S::Queued } });
    }
}
