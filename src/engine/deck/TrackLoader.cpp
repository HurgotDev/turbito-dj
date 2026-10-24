#include "engine/deck/TrackLoader.h"

#include <juce_audio_formats/juce_audio_formats.h>

std::unique_ptr<Track> loadTrack(const juce::File& file)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));

    if (reader == nullptr || reader->numChannels == 0 || reader->lengthInSamples == 0)
        return nullptr;

    const auto length = static_cast<int>(reader->lengthInSamples);

    auto track = std::make_unique<Track>();
    track->audio.setSize(static_cast<int>(reader->numChannels), length);

    if (! reader->read(&track->audio, 0, length, 0, true, true))
        return nullptr;

    track->sampleRate = reader->sampleRate;
    track->name = file.getFileNameWithoutExtension();
    return track;
}
