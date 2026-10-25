#include <catch2/catch_test_macros.hpp>

#include "library/scan/FolderScanner.h"

TEST_CASE("findAudioFiles returns the audio files of a folder tree, sorted", "[library][scan]")
{
    const auto root = juce::File::getSpecialLocation(juce::File::tempDirectory)
                          .getNonexistentChildFile("turbito_scan", "");
    REQUIRE(root.createDirectory());

    const juce::StringArray all { "b.mp3", "A.WAV", "notes.txt", "._x.mp3", ".hidden.flac",
                                  "sub/c.Flac", "sub/d.aiff", "sub/deeper/e.AIF", "sub/f.ogg",
                                  "sub/deeper/cover.jpg" };
    for (const auto& path : all)
        REQUIRE(root.getChildFile(path).create());

    juce::Array<juce::File> expected;
    for (const auto* path : { "b.mp3", "A.WAV", "sub/c.Flac", "sub/d.aiff", "sub/deeper/e.AIF" })
        expected.add(root.getChildFile(path));
    std::sort(expected.begin(), expected.end(),
              [](const juce::File& a, const juce::File& b) { return a.getFullPathName() < b.getFullPathName(); });

    const auto found = findAudioFiles(root);

    REQUIRE(found.size() == static_cast<size_t>(expected.size()));
    for (size_t i = 0; i < found.size(); ++i)
        CHECK(found[i] == expected[static_cast<int>(i)]);

    root.deleteRecursively();
}
