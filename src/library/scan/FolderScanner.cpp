#include "library/scan/FolderScanner.h"

#include <algorithm>

std::vector<juce::File> findAudioFiles(const juce::File& folder)
{
    std::vector<juce::File> files;
    for (const auto& entry : juce::RangedDirectoryIterator(folder, true, "*", juce::File::findFiles))
    {
        const auto& file = entry.getFile();
        // Hidden files include macOS "._song.mp3" resource forks.
        if (file.hasFileExtension("mp3;wav;flac;aiff;aif") && !file.getFileName().startsWithChar('.'))
            files.push_back(file);
    }

    std::sort(files.begin(), files.end(),
              [](const juce::File& a, const juce::File& b) { return a.getFullPathName() < b.getFullPathName(); });
    return files;
}
