#include "ui/look/Fonts.h"

#include <map>

#include "BinaryData.h"

namespace fonts
{
    namespace
    {
        juce::Typeface::Ptr load(const char* data, int size)
        {
            return juce::Typeface::createSystemTypefaceFor(data, static_cast<size_t>(size));
        }

        // A Font can't carry both a typeface and variable settings, so each
        // weight is its own typeface clone with the wght axis baked in.
        juce::Typeface::Ptr withWeight(const juce::Typeface::Ptr& base,
                                       std::map<int, juce::Typeface::Ptr>& cache, int weight)
        {
            auto& typeface = cache[weight];

            if (typeface == nullptr)
            {
                const juce::FontVariableSetting setting { "wght", static_cast<float>(weight) };
                typeface = base->cloneWithVariableSettings({ &setting, 1 });
                jassert(typeface != nullptr);
            }

            return typeface;
        }

        juce::Font fromTypeface(const juce::Typeface::Ptr& typeface, float size)
        {
            return juce::FontOptions(typeface).withPointHeight(size);
        }
    }

    juce::Typeface::Ptr uiTypeface(int weight)
    {
        static const auto base = load(BinaryData::InterVariable_ttf,
                                      BinaryData::InterVariable_ttfSize);
        static std::map<int, juce::Typeface::Ptr> cache;
        return withWeight(base, cache, weight);
    }

    juce::Typeface::Ptr monoTypeface(int weight)
    {
        static const auto base = load(BinaryData::JetBrainsMonoVariable_ttf,
                                      BinaryData::JetBrainsMonoVariable_ttfSize);
        static std::map<int, juce::Typeface::Ptr> cache;
        return withWeight(base, cache, weight);
    }

    juce::Font ui(float size, int weight)
    {
        return fromTypeface(uiTypeface(weight), size);
    }

    juce::Font mono(float size, int weight)
    {
        return fromTypeface(monoTypeface(weight), size);
    }

    juce::Font logo(float size)
    {
        static const auto typeface = load(BinaryData::TitanOneRegular_ttf,
                                          BinaryData::TitanOneRegular_ttfSize);
        return fromTypeface(typeface, size);
    }

    juce::Font tracked(const juce::Font& font, float em)
    {
        return font.withExtraKerningFactor(em * font.getHeightInPoints() / font.getHeight());
    }
}
