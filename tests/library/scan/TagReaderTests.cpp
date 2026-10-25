#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <juce_audio_formats/juce_audio_formats.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#include "library/scan/TagReader.h"

using Catch::Matchers::WithinAbs;

namespace
{
    struct TempDir
    {
        juce::File dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                             .getNonexistentChildFile("turbito_tags", "");

        TempDir() { dir.createDirectory(); }
        ~TempDir() { dir.deleteRecursively(); }
    };

    void writeSilentWav(const juce::File& file, double seconds)
    {
        constexpr double rate = 44100.0;
        juce::AudioBuffer<float> silence(2, static_cast<int>(seconds * rate));
        silence.clear();

        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> out = file.createOutputStream();
        REQUIRE(out != nullptr);
        auto writer = wav.createWriterFor(out, juce::AudioFormatWriterOptions {}
                                                   .withSampleRate(rate)
                                                   .withNumChannels(2)
                                                   .withBitsPerSample(16));
        REQUIRE(writer != nullptr);
        REQUIRE(writer->writeFromAudioSampleBuffer(silence, 0, silence.getNumSamples()));
    }
}

TEST_CASE("readTags reads duration and file-name title from the test WAVs", "[library][scan]")
{
    const auto [name, seconds] = GENERATE(table<const char*, double>({
        { "beat_124bpm_30s", 30.0 },
        { "beat_126bpm_30s", 30.0 },
        { "silencios_120bpm", 27.0 },
        { "tono_la_menor_8A_120bpm", 32.0 },
        { "placa_air_horn", 1.6 },
    }));

    const juce::File file(juce::String(TURBITO_SOURCE_DIR "/test-audio/") + name + ".wav");
    const auto tags = readTags(file);

    REQUIRE(tags.has_value());
    CHECK_THAT(tags->duration, WithinAbs(seconds, 0.01));
    CHECK(tags->title == juce::String(name));
    CHECK(tags->artist.isEmpty());
}

TEST_CASE("readTags returns the title and artist stored in the file", "[library][scan]")
{
    TempDir temp;
    const auto file = temp.dir.getChildFile("tagged.wav");
    writeSilentWav(file, 0.5);

    const auto artist = juce::String(juce::CharPointer_UTF8("Lisandro Meza \xe2\x80\x93 Se\xc3\xb1or"));
    {
#if JUCE_WINDOWS
        TagLib::FileRef ref(file.getFullPathName().toWideCharPointer());
#else
        TagLib::FileRef ref(file.getFullPathName().toRawUTF8());
#endif
        REQUIRE(!ref.isNull());
        ref.tag()->setTitle(TagLib::String("  La Burrita  ", TagLib::String::UTF8));
        ref.tag()->setArtist(TagLib::String(artist.toRawUTF8(), TagLib::String::UTF8));
        REQUIRE(ref.save());
    }

    const auto tags = readTags(file);

    REQUIRE(tags.has_value());
    CHECK(tags->title == juce::String("La Burrita"));
    CHECK(tags->artist == artist);
    CHECK_THAT(tags->duration, WithinAbs(0.5, 0.01));
}

TEST_CASE("readTags returns nullopt for a file that is not audio", "[library][scan]")
{
    TempDir temp;
    const auto file = temp.dir.getChildFile("junk.mp3");
    REQUIRE(file.replaceWithText("this is not an mp3, just some text pretending to be one"));

    CHECK_FALSE(readTags(file).has_value());
    CHECK_FALSE(readTags(temp.dir.getChildFile("missing.wav")).has_value());
}
