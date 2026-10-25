#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/library/AutoMixPanel.h"
#include "ui/look/Tokens.h"
#include "ui/widgets/PaintHelpers.h"

namespace
{
    juce::String utf8(const char* text) { return juce::String::fromUTF8(text); }

    // The mockup's queue.
    std::vector<QueueItem> mockupQueue()
    {
        return { { "Calor de Verano", "Nativa Club", "124", "9B", "3:58", QueueStatus::Played },
                 { utf8("Luces de Neón"), "Varela & Sonido Norte", "124", "8A", "6:12",
                   QueueStatus::Playing },
                 { "Medianoche en Polanco", "Kiro Beat", "126", "5A", "5:48", QueueStatus::Next },
                 { "Fuego Lento", "La Brava", "122", "11B", "4:55", QueueStatus::Queued },
                 { "Pulso", "Dani Ortega", "128", "2A", "6:30", QueueStatus::Queued },
                 { "Calor de Verano", "Nativa Club", "124", "9B", "3:58", QueueStatus::Queued },
                 { "Ritmo Callejero", "Los Bajos", "126", "7A", "5:21", QueueStatus::Queued } };
    }

    template <typename T>
    T& child(juce::Component& parent, const juce::String& title = {})
    {
        for (auto* c : parent.getChildren())
            if (auto* found = dynamic_cast<T*>(c); found != nullptr
                && (title.isEmpty() || c->getTitle() == title))
                return *found;

        FAIL("No such child " << title);
        throw std::logic_error("unreachable");
    }

    juce::Button& removeButton(juce::ListBox& list, int row)
    {
        auto* buttons = list.getComponentForRowNumber(row);
        REQUIRE(buttons != nullptr);
        return child<juce::TextButton>(*buttons);
    }
}

TEST_CASE("Auto Mix summary reads like the mockup", "[ui][automix]")
{
    AutoMixOptions options;
    CHECK(autoMixSummary(options) == utf8("Decks A/B · Sin silencios · BPM emparejado · 16 beats"));

    options.trimSilence = false;
    options.matchBpm = false;
    const auto normal = autoMixSummary(options);
    CHECK(normal.contains("Normal"));
    CHECK(normal.endsWith("8 s"));

    options.singleDeck = true;
    options.random = true;
    options.autoRemove = true;
    CHECK(autoMixSummary(options) == utf8("Un solo deck · Normal · 8 s · Aleatorio · Auto quitar"));
}

TEST_CASE("Pending count skips played tracks and duplicates flag later copies", "[ui][automix]")
{
    const auto queue = mockupQueue();

    CHECK(pendingCount(queue) == 6);
    CHECK(duplicateFlags(queue) == std::vector<bool> { false, false, false, false, false, true, false });
}

TEST_CASE("addTrack appends a queued row and × removes rows but the playing one",
          "[ui][automix]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AutoMixPanel panel;
    panel.setBounds(0, 0, 400, 240);
    panel.setQueue(mockupQueue());

    panel.addTrack({ juce::File("/music/Nuevo.wav"), "Nuevo", {}, {}, {}, {} });
    REQUIRE(panel.getQueue().size() == 8);
    CHECK(panel.getQueue().back().title == "Nuevo");
    CHECK(panel.getQueue().back().artist == utf8("—"));
    CHECK(panel.getQueue().back().status == QueueStatus::Queued);

    auto& list = child<juce::ListBox>(panel);
    CHECK(removeButton(list, 0).isEnabled());
    CHECK_FALSE(removeButton(list, 1).isEnabled());

    removeButton(list, 0).onClick();
    REQUIRE(panel.getQueue().size() == 7);
    CHECK(panel.getQueue().front().status == QueueStatus::Playing);
    CHECK_FALSE(removeButton(list, 0).isEnabled());
}

TEST_CASE("The switch toggles Activo and Pausado", "[ui][automix]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AutoMixPanel panel;
    auto& toggle = child<juce::Button>(panel, "Auto Mix activo");
    CHECK(panel.isActive());
    CHECK(toggle.getButtonText() == "Activo");

    toggle.onClick();
    CHECK_FALSE(panel.isActive());
    CHECK(toggle.getButtonText() == "Pausado");
}

TEST_CASE("Hovering a menu button shows its tooltip above it", "[ui][automix]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AutoMixPanel panel;
    panel.setBounds(0, 0, 400, 240);
    const auto name = utf8("Transición");
    auto& button = child<juce::Button>(panel, name);

    button.setState(juce::Button::buttonOver);

    juce::Component* tip = nullptr;
    for (auto* c : panel.getChildren())
        if (dynamic_cast<juce::Button*>(c) == nullptr && c->isVisible() && c->getTitle() == name)
            tip = c;

    REQUIRE(tip != nullptr);
    CHECK(tip->getBottom() == button.getY() - 6);
    CHECK(std::abs(tip->getBounds().getCentreX() - button.getBounds().getCentreX()) <= 1);

    button.setState(juce::Button::buttonNormal);
    CHECK_FALSE(tip->isVisible());
}

TEST_CASE("Collapsing shrinks the library's side panel to the strip", "[ui][automix]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    AutoMixPanel panel;
    std::vector<bool> changes;

    // AppView's wiring, by hand.
    library.onAddToAutoMix = [&](const LibraryTrack& track) { panel.addTrack(track); };
    panel.onCollapsedChanged = [&](bool collapsed)
    {
        changes.push_back(collapsed);
        library.setSidePanelCollapsed(collapsed);
    };

    library.setBounds(0, 0, 1420, 266);
    library.setSidePanel(&panel);
    CHECK(panel.getWidth() == 400);

    library.onAddToAutoMix({ juce::File("/music/Pulso.mp3"), "Pulso", {}, {}, {}, {} });
    CHECK(panel.getQueue().size() == 1);

    child<juce::Button>(panel, "Contraer el panel de Auto Mix").onClick();
    CHECK(panel.isCollapsed());
    CHECK(changes == std::vector<bool> { true });
    CHECK(panel.getWidth() == 44);
    CHECK(panel.getRight() == library.getWidth() - 13);

    juce::Component* sideResizer = nullptr;
    for (auto* c : library.getChildren())
        if (dynamic_cast<PanelResizer*>(c) != nullptr)
            sideResizer = c;  // the second one is the side panel's
    REQUIRE(sideResizer != nullptr);
    CHECK_FALSE(sideResizer->isVisible());

    panel.setCollapsed(false);
    CHECK(changes == std::vector<bool> { true, false });
    CHECK(panel.getWidth() == 400);
    CHECK(sideResizer->isVisible());

    library.setSidePanel(nullptr);
}

TEST_CASE("The playing row's badge is deckA", "[ui][automix]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AutoMixPanel panel;
    panel.setQueue(mockupQueue());

    const auto image = paintWidget(panel, 400, 240);
    const auto list = child<juce::ListBox>(panel).getBounds();
    // Second row, above the text in its 18-high badge (row content starts 6 down).
    const auto y = list.getY() + 44 + 6 + 3;

    auto badgePixels = 0;
    for (auto x = list.getX(); x < list.getRight(); ++x)
        badgePixels += image.getPixelAt(x, y) == juce::Colour(tokens::deckA) ? 1 : 0;

    CHECK(badgePixels > 20);
    CHECK(image.getPixelAt(list.getX() + 3, y) == juce::Colour(tokens::rowActive));
}
