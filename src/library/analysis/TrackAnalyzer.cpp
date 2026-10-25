#include "library/analysis/TrackAnalyzer.h"

#include "library/analysis/BeatAnalyzer.h"
#include "library/analysis/KeyAnalyzer.h"
#include "library/analysis/WaveformBuilder.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <vector>

std::optional<TrackAnalysis> analyzeTrack(const juce::File& file, const std::function<bool()>& shouldExit)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));

    if (reader == nullptr || reader->numChannels == 0 || reader->lengthInSamples == 0 || reader->sampleRate <= 0)
        return std::nullopt;

    constexpr int blockSize = 8192;
    const int numChannels = static_cast<int>(reader->numChannels);
    juce::AudioBuffer<float> block(numChannels, blockSize);
    std::vector<float> mono(blockSize);

    BeatAnalyzer beats(reader->sampleRate);
    KeyAnalyzer key(reader->sampleRate);
    WaveformBuilder waveform(reader->sampleRate);

    for (juce::int64 pos = 0; pos < reader->lengthInSamples; pos += blockSize)
    {
        if (shouldExit && shouldExit())
            return std::nullopt;

        const auto n = static_cast<int>(std::min<juce::int64>(blockSize, reader->lengthInSamples - pos));
        if (! reader->read(&block, 0, n, pos, true, true))
            return std::nullopt;

        juce::FloatVectorOperations::copy(mono.data(), block.getReadPointer(0), n);
        for (int ch = 1; ch < numChannels; ++ch)
            juce::FloatVectorOperations::add(mono.data(), block.getReadPointer(ch), n);
        juce::FloatVectorOperations::multiply(mono.data(), 1.0f / static_cast<float>(numChannels), n);

        beats.process(mono.data(), n);
        key.process(mono.data(), n);
        waveform.process(mono.data(), n);
    }

    const auto grid = beats.finish();
    TrackAnalysis result;
    result.bpm = grid.bpm;
    result.firstBeat = grid.firstBeat;
    result.camelot = key.finish();
    result.waveform = waveform.finish();
    return result;
}
