#include "library/Query.h"

#include <catch2/catch_test_macros.hpp>

namespace
{
    LibraryRecord makeTrack(juce::String title,
                            juce::String artist,
                            std::optional<double> bpm = std::nullopt,
                            juce::String camelot = {},
                            double duration = 0.0,
                            juce::int64 id = 0)
    {
        LibraryRecord r;
        r.id = id;
        r.title = title;
        r.artist = artist;
        r.bpm = bpm;
        r.camelot = camelot;
        r.duration = duration;
        r.analyzed = bpm.has_value();
        return r;
    }

    std::vector<juce::int64> ids(const std::vector<LibraryRecord>& records)
    {
        std::vector<juce::int64> out;
        for (const auto& r : records)
            out.push_back(r.id);
        return out;
    }
} // namespace

TEST_CASE("matchesSearch: empty text matches everything", "[query]")
{
    const auto t = makeTrack("La Pollera Colorá", "Wilson Choperena");
    CHECK(matchesSearch(t, ""));
    CHECK(matchesSearch(t, "   "));
}

TEST_CASE("matchesSearch: every word must appear, in any field", "[query]")
{
    const auto t = makeTrack(juce::String::fromUTF8("El Señor de la Cumbia"), "Los Corraleros", 124.0, "8A");

    CHECK(matchesSearch(t, "cumbia"));
    CHECK(matchesSearch(t, "CUMBIA corraleros"));
    CHECK(matchesSearch(t, "senor"));
    CHECK(matchesSearch(t, juce::String::fromUTF8("SEÑOR")));
    CHECK_FALSE(matchesSearch(t, "cumbia salsa"));
    CHECK_FALSE(matchesSearch(t, "champeta"));
}

TEST_CASE("matchesSearch: accents fold on both sides", "[query]")
{
    const auto t = makeTrack(juce::String::fromUTF8("Pingüino Ça Va Ánimo"), "Ivan");
    CHECK(matchesSearch(t, "pinguino"));
    CHECK(matchesSearch(t, "ca va animo"));
    CHECK(matchesSearch(makeTrack("Ivan", "x"), juce::String::fromUTF8("iván")));
    CHECK(matchesSearch(makeTrack(juce::String::fromUTF8("Łódź Poznań Ōsaka"), "x"), "odz poznan osaka"));
    CHECK(matchesSearch(makeTrack("Poznan", "x"), juce::String::fromUTF8("POZNAŃ")));
}

TEST_CASE("matchesSearch: BPM as the table shows it and Camelot key", "[query]")
{
    const auto t = makeTrack("Tema", "Artista", 124.0, "8A");
    CHECK(matchesSearch(t, "124"));
    CHECK(matchesSearch(t, "124.0"));
    CHECK(matchesSearch(t, "8a"));
    CHECK(matchesSearch(t, "8A tema"));
    CHECK_FALSE(matchesSearch(t, "125"));
    CHECK_FALSE(matchesSearch(t, "8b"));
    CHECK_FALSE(matchesSearch(makeTrack("Tema", "Artista"), "124"));
}

TEST_CASE("matchesSearch: words do not match across fields", "[query]")
{
    const auto t = makeTrack("Abc", "Def");
    CHECK_FALSE(matchesSearch(t, "cde"));
}

TEST_CASE("passesFilter: all and unanalyzed", "[query]")
{
    const auto analyzed = makeTrack("A", "x", 120.0, "8A");
    const auto raw = makeTrack("B", "x");

    CHECK(passesFilter(analyzed, LibraryFilter::all, std::nullopt, {}));
    CHECK(passesFilter(raw, LibraryFilter::all, 120.0, "8A"));
    CHECK(passesFilter(raw, LibraryFilter::unanalyzed, std::nullopt, {}));
    CHECK_FALSE(passesFilter(analyzed, LibraryFilter::unanalyzed, std::nullopt, {}));
}

TEST_CASE("passesFilter: BPM within 3 of deck A", "[query]")
{
    const auto f = LibraryFilter::bpmNearDeckA;
    CHECK(passesFilter(makeTrack("a", "x", 120.0), f, 123.0, {}));
    CHECK(passesFilter(makeTrack("a", "x", 126.0), f, 123.0, {}));
    CHECK(passesFilter(makeTrack("a", "x", 123.0), f, 123.0, {}));
    CHECK_FALSE(passesFilter(makeTrack("a", "x", 126.5), f, 123.0, {}));
    CHECK_FALSE(passesFilter(makeTrack("a", "x", 119.9), f, 123.0, {}));
    CHECK_FALSE(passesFilter(makeTrack("a", "x"), f, 123.0, {}));

    SECTION("no deck A BPM passes everything")
    {
        CHECK(passesFilter(makeTrack("a", "x", 90.0), f, std::nullopt, {}));
        CHECK(passesFilter(makeTrack("a", "x"), f, std::nullopt, {}));
    }
}

TEST_CASE("passesFilter: key compatible with deck A", "[query]")
{
    const auto f = LibraryFilter::keyCompatibleWithDeckA;
    CHECK(passesFilter(makeTrack("a", "x", 1.0, "9A"), f, std::nullopt, "8A"));
    CHECK(passesFilter(makeTrack("a", "x", 1.0, "8B"), f, std::nullopt, "8A"));
    CHECK_FALSE(passesFilter(makeTrack("a", "x", 1.0, "3A"), f, std::nullopt, "8A"));
    CHECK_FALSE(passesFilter(makeTrack("a", "x"), f, std::nullopt, "8A"));

    SECTION("no deck A key passes everything")
    {
        CHECK(passesFilter(makeTrack("a", "x", 1.0, "3A"), f, std::nullopt, {}));
        CHECK(passesFilter(makeTrack("a", "x"), f, std::nullopt, {}));
    }
}

TEST_CASE("isKeyCompatible: full Camelot wheel", "[query]")
{
    for (int n = 1; n <= 12; ++n)
    {
        const auto up = n % 12 + 1;
        const auto down = (n + 10) % 12 + 1;

        for (const auto* letter : { "A", "B" })
        {
            const auto code = juce::String(n) + letter;
            const auto other = juce::String(n) + (juce::String(letter) == "A" ? "B" : "A");

            INFO(code.toStdString());
            CHECK(isKeyCompatible(code, code));
            CHECK(isKeyCompatible(code, juce::String(up) + letter));
            CHECK(isKeyCompatible(code, juce::String(down) + letter));
            CHECK(isKeyCompatible(code, other));
            CHECK_FALSE(isKeyCompatible(code, juce::String((n + 1) % 12 + 1) + letter));
            CHECK_FALSE(isKeyCompatible(code, juce::String(up) + (juce::String(letter) == "A" ? "B" : "A")));
        }
    }
}

TEST_CASE("isKeyCompatible: wrap, case and invalid codes", "[query]")
{
    CHECK(isKeyCompatible("12A", "1A"));
    CHECK(isKeyCompatible("1B", "12B"));
    CHECK(isKeyCompatible("8A", "8B"));
    CHECK(isKeyCompatible("8a", "9A"));
    CHECK_FALSE(isKeyCompatible("12A", "1B"));
    CHECK_FALSE(isKeyCompatible("8A", "2A"));
    CHECK_FALSE(isKeyCompatible("", "8A"));
    CHECK_FALSE(isKeyCompatible("8A", ""));
    CHECK_FALSE(isKeyCompatible("13A", "1A"));
    CHECK_FALSE(isKeyCompatible("8C", "8A"));
}

TEST_CASE("sortRecords: title and artist natural, case and accent insensitive", "[query]")
{
    std::vector<LibraryRecord> r { makeTrack("track 10", "b", {}, {}, 0, 1),
                                   makeTrack("Track 2", juce::String::fromUTF8("Ñu"), {}, {}, 0, 2),
                                   makeTrack(juce::String::fromUTF8("Árbol"), "Zeta", {}, {}, 0, 3),
                                   makeTrack("banda", "alfa", {}, {}, 0, 4) };

    sortRecords(r, LibraryColumn::title, true);
    CHECK(ids(r) == std::vector<juce::int64> { 3, 4, 2, 1 });
    sortRecords(r, LibraryColumn::title, false);
    CHECK(ids(r) == std::vector<juce::int64> { 1, 2, 4, 3 });

    sortRecords(r, LibraryColumn::artist, true);
    CHECK(ids(r) == std::vector<juce::int64> { 4, 1, 2, 3 });
    sortRecords(r, LibraryColumn::artist, false);
    CHECK(ids(r) == std::vector<juce::int64> { 3, 2, 1, 4 });
}

TEST_CASE("sortRecords: BPM numeric with missing last both ways", "[query]")
{
    std::vector<LibraryRecord> r { makeTrack("a", "x", {}, {}, 0, 1),
                                   makeTrack("b", "x", 128.0, {}, 0, 2),
                                   makeTrack("c", "x", 95.5, {}, 0, 3),
                                   makeTrack("d", "x", {}, {}, 0, 4),
                                   makeTrack("e", "x", 100.0, {}, 0, 5) };

    sortRecords(r, LibraryColumn::bpm, true);
    CHECK(ids(r) == std::vector<juce::int64> { 3, 5, 2, 1, 4 });
    sortRecords(r, LibraryColumn::bpm, false);
    CHECK(ids(r) == std::vector<juce::int64> { 2, 5, 3, 1, 4 });
}

TEST_CASE("sortRecords: key by number then letter, missing last", "[query]")
{
    std::vector<LibraryRecord> r { makeTrack("a", "x", {}, "10A", 0, 1),
                                   makeTrack("b", "x", {}, "", 0, 2),
                                   makeTrack("c", "x", {}, "2B", 0, 3),
                                   makeTrack("d", "x", {}, "2A", 0, 4),
                                   makeTrack("e", "x", {}, "9B", 0, 5) };

    sortRecords(r, LibraryColumn::key, true);
    CHECK(ids(r) == std::vector<juce::int64> { 4, 3, 5, 1, 2 });
    sortRecords(r, LibraryColumn::key, false);
    CHECK(ids(r) == std::vector<juce::int64> { 1, 5, 3, 4, 2 });
}

TEST_CASE("sortRecords: duration numeric", "[query]")
{
    std::vector<LibraryRecord> r { makeTrack("a", "x", {}, {}, 200.0, 1),
                                   makeTrack("b", "x", {}, {}, 65.0, 2),
                                   makeTrack("c", "x", {}, {}, 300.5, 3) };

    sortRecords(r, LibraryColumn::duration, true);
    CHECK(ids(r) == std::vector<juce::int64> { 2, 1, 3 });
    sortRecords(r, LibraryColumn::duration, false);
    CHECK(ids(r) == std::vector<juce::int64> { 3, 1, 2 });
}

TEST_CASE("sortRecords: stable for equal keys", "[query]")
{
    std::vector<LibraryRecord> r { makeTrack("a", "x", 120.0, {}, 0, 1),
                                   makeTrack("b", "x", 120.0, {}, 0, 2),
                                   makeTrack("c", "x", {}, {}, 0, 3),
                                   makeTrack("d", "x", 120.0, {}, 0, 4),
                                   makeTrack("e", "x", {}, {}, 0, 5) };

    sortRecords(r, LibraryColumn::bpm, true);
    CHECK(ids(r) == std::vector<juce::int64> { 1, 2, 4, 3, 5 });
    sortRecords(r, LibraryColumn::bpm, false);
    CHECK(ids(r) == std::vector<juce::int64> { 1, 2, 4, 3, 5 });
    sortRecords(r, LibraryColumn::artist, false);
    CHECK(ids(r) == std::vector<juce::int64> { 1, 2, 4, 3, 5 });
}
