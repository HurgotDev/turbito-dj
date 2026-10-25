#include "library/scan/TagReader.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

namespace
{
    juce::String toJuce(const TagLib::String& s)
    {
        return juce::String(juce::CharPointer_UTF8(s.toCString(true))).trim();
    }
}

std::optional<TrackTags> readTags(const juce::File& file)
{
    // AudioFormatManager is not thread-safe; one per scanning thread.
    thread_local const auto formats = [] {
        auto manager = std::make_unique<juce::AudioFormatManager>();
        manager->registerBasicFormats();
        return manager;
    }();

    const std::unique_ptr<juce::AudioFormatReader> reader(formats->createReaderFor(file));
    if (reader == nullptr || reader->sampleRate <= 0)
        return std::nullopt;

    TrackTags tags;
    tags.duration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

#if JUCE_WINDOWS
    const TagLib::FileRef ref(file.getFullPathName().toWideCharPointer(), false);
#else
    const TagLib::FileRef ref(file.getFullPathName().toRawUTF8(), false);
#endif
    if (!ref.isNull() && ref.tag() != nullptr)
    {
        tags.title = toJuce(ref.tag()->title());
        tags.artist = toJuce(ref.tag()->artist());
    }

    if (tags.title.isEmpty())
        tags.title = file.getFileNameWithoutExtension();

    return tags;
}
