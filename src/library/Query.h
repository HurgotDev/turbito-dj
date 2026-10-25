#pragma once

#include "library/Track.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

// Index order matches strings::libraryFilters.
enum class LibraryFilter
{
    all,
    bpmNearDeckA,
    keyCompatibleWithDeckA,
    unanalyzed
};

enum class LibraryColumn
{
    title,
    artist,
    bpm,
    key,
    duration
};

namespace query
{
    // Lower case without accents: "Señor" -> "senor".
    inline juce::String fold(const juce::String& text)
    {
        // shortcut: Latin-1 and the common Latin Extended-A letters only, extend if users hit others.
        static constexpr struct
        {
            juce::juce_wchar from;
            char to;
        } table[] = {
            { 0xE0, 'a' }, { 0xE1, 'a' }, { 0xE2, 'a' }, { 0xE3, 'a' }, { 0xE4, 'a' }, { 0xE5, 'a' },
            { 0x101, 'a' }, { 0x103, 'a' }, { 0x105, 'a' }, { 0xE7, 'c' }, { 0x107, 'c' }, { 0x10D, 'c' },
            { 0xE8, 'e' }, { 0xE9, 'e' }, { 0xEA, 'e' }, { 0xEB, 'e' }, { 0x113, 'e' }, { 0x117, 'e' },
            { 0x119, 'e' }, { 0x11B, 'e' }, { 0xEC, 'i' }, { 0xED, 'i' }, { 0xEE, 'i' }, { 0xEF, 'i' },
            { 0x12B, 'i' }, { 0x12F, 'i' }, { 0xF1, 'n' }, { 0x144, 'n' }, { 0x148, 'n' }, { 0xF2, 'o' },
            { 0xF3, 'o' }, { 0xF4, 'o' }, { 0xF5, 'o' }, { 0xF6, 'o' }, { 0xF8, 'o' }, { 0x14D, 'o' },
            { 0x151, 'o' }, { 0xF9, 'u' }, { 0xFA, 'u' }, { 0xFB, 'u' }, { 0xFC, 'u' }, { 0x16B, 'u' },
            { 0x16F, 'u' }, { 0x171, 'u' }, { 0x173, 'u' }, { 0xFD, 'y' }, { 0xFF, 'y' }, { 0x142, 'l' },
            { 0x15B, 's' }, { 0x161, 's' }, { 0x17A, 'z' }, { 0x17C, 'z' }, { 0x17E, 'z' },
        };

        juce::String out;
        out.preallocateBytes(text.getNumBytesAsUTF8());

        for (auto c : text.toLowerCase())
        {
            // toLowerCase leaves non-ASCII as is: upper case Latin-1 is lower - 0x20, Extended-A is lower - 1.
            if (c >= 0xC0 && c <= 0xDE && c != 0xD7)
                c += 0x20;
            else if (const auto oddUpper = (c >= 0x139 && c <= 0x148) || (c >= 0x179 && c <= 0x17E);
                     c >= 0x100 && c <= 0x177 && c != 0x138 && c != 0x149 && (c % 2 == 1) == oddUpper)
                c += 1;

            const auto* hit = std::find_if(std::begin(table), std::end(table), [c](const auto& e) { return e.from == c; });
            out += hit != std::end(table) ? static_cast<juce::juce_wchar>(hit->to) : c;
        }

        return out;
    }

    // Camelot "8A" -> {8, 'A'}; nullopt when it is not a valid code.
    inline std::optional<std::pair<int, juce::juce_wchar>> parseCamelot(const juce::String& code)
    {
        const auto trimmed = code.trim().toUpperCase();
        const auto letter = trimmed.getLastCharacter();
        const auto digits = trimmed.dropLastCharacters(1);

        if ((letter != 'A' && letter != 'B') || digits.isEmpty() || !digits.containsOnly("0123456789"))
            return std::nullopt;

        const auto number = digits.getIntValue();
        if (number < 1 || number > 12)
            return std::nullopt;

        return std::pair { number, letter };
    }
} // namespace query

// Empty text matches everything; otherwise every word must appear in title, artist, BPM or key.
inline bool matchesSearch(const LibraryRecord& track, const juce::String& text)
{
    juce::StringArray words;
    words.addTokens(query::fold(text), true);
    words.removeEmptyStrings();

    // "\n" never appears inside a word, so a word cannot match across two fields.
    const auto haystack = query::fold(track.title + "\n" + track.artist + "\n"
                                      + (track.bpm ? juce::String(*track.bpm, 1) : juce::String()) + "\n"
                                      + track.camelot);

    return std::all_of(words.begin(), words.end(), [&](const auto& w) { return haystack.contains(w); });
}

// Camelot wheel: same code, same letter one step away (12 wraps to 1), or same number other letter.
inline bool isKeyCompatible(const juce::String& a, const juce::String& b)
{
    const auto ka = query::parseCamelot(a);
    const auto kb = query::parseCamelot(b);
    if (!ka || !kb)
        return false;

    if (ka->first == kb->first)
        return true;

    const auto step = (ka->first - kb->first + 12) % 12;
    return ka->second == kb->second && (step == 1 || step == 11);
}

// Without deck A data the filter passes everything: an empty list would look like a broken library.
inline bool passesFilter(const LibraryRecord& track,
                         LibraryFilter filter,
                         std::optional<double> deckABpm,
                         const juce::String& deckACamelot)
{
    switch (filter)
    {
        case LibraryFilter::all:
            return true;
        case LibraryFilter::bpmNearDeckA:
            return !deckABpm || (track.bpm && std::abs(*track.bpm - *deckABpm) <= 3.0);
        case LibraryFilter::keyCompatibleWithDeckA:
            return deckACamelot.isEmpty() || isKeyCompatible(track.camelot, deckACamelot);
        case LibraryFilter::unanalyzed:
            return !track.analyzed;
    }

    return true;
}

// Stable sort; tracks without BPM or key stay last in both directions.
inline void sortRecords(std::vector<LibraryRecord>& records, LibraryColumn column, bool ascending)
{
    const auto order = [ascending](auto a, auto b) { return ascending ? a < b : b < a; };

    std::stable_sort(records.begin(), records.end(), [&](const LibraryRecord& a, const LibraryRecord& b) {
        switch (column)
        {
            case LibraryColumn::title:
                return order(query::fold(a.title).compareNatural(query::fold(b.title)), 0);
            case LibraryColumn::artist:
                return order(query::fold(a.artist).compareNatural(query::fold(b.artist)), 0);
            case LibraryColumn::bpm:
                if (!a.bpm || !b.bpm)
                    return a.bpm.has_value() && !b.bpm.has_value();
                return order(*a.bpm, *b.bpm);
            case LibraryColumn::key:
            {
                const auto ka = query::parseCamelot(a.camelot);
                const auto kb = query::parseCamelot(b.camelot);
                if (!ka || !kb)
                    return ka.has_value() && !kb.has_value();
                return order(*ka, *kb);
            }
            case LibraryColumn::duration:
                return order(a.duration, b.duration);
        }

        return false;
    });
}
