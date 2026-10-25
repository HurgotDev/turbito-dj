#include <catch2/catch_test_macros.hpp>
#include <juce_core/juce_core.h>

#include <cstdint>

#include "BinaryData.h"
#include "ui/look/Tokens.h"

TEST_CASE("Tokens.h matches design/tokens.json", "[tokens]")
{
    const auto json = juce::File(TURBITO_SOURCE_DIR "/design/tokens.json")
                          .loadFileAsString();
    const auto root = juce::JSON::parse(json);
    REQUIRE(! root.isVoid());

    const auto reference = root["window"]["reference"];
    CHECK(static_cast<int>(reference[0]) == tokens::windowReference.width);
    CHECK(static_cast<int>(reference[1]) == tokens::windowReference.height);

    const auto minimum = root["window"]["minimum"];
    CHECK(static_cast<int>(minimum[0]) == tokens::windowMinimum.width);
    CHECK(static_cast<int>(minimum[1]) == tokens::windowMinimum.height);

    const auto bg = root["color"]["bg"].toString();
    CHECK(tokens::bg == (0xff000000u | static_cast<std::uint32_t>(bg.substring(1).getHexValue32())));
}

TEST_CASE("Generated tokens keep their spot values", "[tokens]")
{
    CHECK(tokens::playhead == 0xffFFFFFFu);
    CHECK(tokens::hotCues.size() == 7u);
    CHECK(tokens::layoutMixerWidth == 300);
}

TEST_CASE("Embedded fonts and logo are not empty", "[tokens][assets]")
{
    CHECK(BinaryData::InterVariable_ttfSize > 0);
    CHECK(BinaryData::JetBrainsMonoVariable_ttfSize > 0);
    CHECK(BinaryData::TitanOneRegular_ttfSize > 0);
    CHECK(BinaryData::turbitohorizontal_svgSize > 0);
}
