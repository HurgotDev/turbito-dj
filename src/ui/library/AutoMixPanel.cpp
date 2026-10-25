#include "ui/library/AutoMixPanel.h"

#include <algorithm>
#include <set>

#include "ui/Layout.h"
#include "ui/Strings.h"
#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    // Mockup geometry, not tokens.
    constexpr float panelRadius = 10.0f;
    constexpr int inset = 1 + 10;  // border + padding
    constexpr int rowGap = 8;
    constexpr int headerHeight = 26;
    constexpr int headerGap = 6;
    constexpr int switchPadding = 8;
    constexpr int trackWidth = 24;
    constexpr int trackHeight = 12;
    constexpr int menuButtonSize = 30;
    constexpr int menuButtonGap = 4;
    constexpr int menuWidth = 232;
    constexpr int menuOffset = 6;
    constexpr int tooltipGap = 6;
    constexpr float queueRadius = 8.0f;

    constexpr int rowHeight = 44;  // 6 + 18 + 13 + 6 + 1px border, measured in the PNG
    constexpr int rowPadding = 8;
    constexpr int numberWidth = 16;
    constexpr int titleLine = 18;
    constexpr int titleGap = 6;
    constexpr int badgePadding = 6;
    constexpr int removeSize = 24;

    constexpr int stripPadding = 12;
    constexpr int stripGap = 12;

    juce::Colour colour(std::uint32_t argb) { return juce::Colour(argb); }
    juce::String utf8(const char* text) { return juce::String::fromUTF8(text); }

    juce::Font smallFont(int weight = 400)
    {
        return fonts::ui(static_cast<float>(tokens::fontSizeLabel), weight);
    }

    // Lucide-style 24-unit icons from the mockup, stroked 2 units wide.
    juce::Path icon(const char* svg)
    {
        return juce::Drawable::parseSVGPath(svg);
    }

    juce::Path decksIcon()
    {
        auto path = icon("M7 12h.01M17 12h.01");
        path.addEllipse(3.0f, 8.0f, 8.0f, 8.0f);
        path.addEllipse(13.0f, 8.0f, 8.0f, 8.0f);
        return path;
    }

    void strokeIcon(juce::Graphics& g, const juce::Path& path, juce::Rectangle<float> box)
    {
        const auto scale = box.getWidth() / 24.0f;
        g.strokePath(path, juce::PathStrokeType(2.0f * scale, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded),
                     juce::AffineTransform::scale(scale).translated(box.getX(), box.getY()));
    }

    struct StatusStyle
    {
        juce::String label;
        std::uint32_t badge, badgeText, text;
    };

    StatusStyle styleOf(QueueStatus status, bool singleDeck)
    {
        const auto index = static_cast<std::size_t>(status);
        const auto label = utf8(strings::queueStatuses[index]);

        switch (status)
        {
            case QueueStatus::Played:  return { label, tokens::empty, tokens::textMuted, tokens::textMuted };
            case QueueStatus::Playing: return { label, tokens::deckA, tokens::bg, tokens::text };
            case QueueStatus::Next:
                return singleDeck ? StatusStyle { utf8(strings::nextSingleDeck), tokens::text,
                                                  tokens::bg, tokens::text }
                                  : StatusStyle { label, tokens::deckB, tokens::bg, tokens::text };
            case QueueStatus::Queued:  break;
        }

        return { label, tokens::empty, tokens::text, tokens::text };
    }

    constexpr int durationsInBeats[] = { 8, 16, 32 };
    constexpr int durationsInSeconds[] = { 4, 8, 12 };
}

juce::String autoMixSummary(const AutoMixOptions& o)
{
    juce::StringArray parts { utf8(strings::summaryDecks[o.singleDeck ? 1 : 0]) };

    if (! o.trimSilence && ! o.matchBpm)
        parts.add(strings::summaryNormal);
    if (o.trimSilence)
        parts.add(strings::summaryTrim);
    if (o.matchBpm)
        parts.add(strings::summaryBpm);

    parts.add(o.matchBpm ? juce::String(o.beats) + strings::beats
                         : juce::String(o.seconds) + strings::summarySeconds);

    if (o.random)
        parts.add(strings::randomOrder);
    if (o.autoRemove)
        parts.add(strings::summaryAutoRemove);

    return parts.joinIntoString(utf8(strings::separator));
}

int pendingCount(const std::vector<QueueItem>& queue)
{
    return static_cast<int>(std::count_if(queue.begin(), queue.end(), [](const QueueItem& item)
                                          { return item.status != QueueStatus::Played; }));
}

std::vector<bool> duplicateFlags(const std::vector<QueueItem>& queue)
{
    std::set<juce::String> seen;
    std::vector<bool> flags;

    for (const auto& item : queue)
        flags.push_back(! seen.insert(item.title).second);

    return flags;
}

//==============================================================================
// Square button with a stroked icon: surface and borderControl, or empty and
// deckA while its menu is open.
class AutoMixPanel::IconButton : public juce::Button
{
public:
    IconButton(const juce::String& title, juce::Path iconToUse, float iconSize, float radius)
        : juce::Button(title), path(std::move(iconToUse)), size(iconSize), corner(radius)
    {
        setTitle(title);
        setWantsKeyboardFocus(false);
    }

    bool open = false;

    void paintButton(juce::Graphics& g, bool, bool) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(colour(open ? tokens::empty : tokens::surface));
        g.fillRoundedRectangle(bounds, corner);
        g.setColour(colour(open ? tokens::deckA : tokens::borderControl));
        g.drawRoundedRectangle(bounds, corner, 1.0f);

        g.setColour(colour(tokens::text));
        strokeIcon(g, path, juce::Rectangle<float>(size, size).withCentre(bounds.getCentre()));
    }

private:
    juce::Path path;
    float size, corner;
};

// Activo/Pausado pill: a 24 × 12 track with an 8px dot, then the label.
class AutoMixPanel::Switch : public juce::Button
{
public:
    Switch() : juce::Button(strings::autoMixSwitch)
    {
        setTitle(strings::autoMixSwitch);
        setWantsKeyboardFocus(false);
    }

    int preferredWidth() const
    {
        return 2 * (1 + switchPadding) + trackWidth + headerGap
             + textWidth(smallFont(600), getButtonText());
    }

    void paintButton(juce::Graphics& g, bool, bool) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        const auto on = getToggleState();

        g.setColour(colour(tokens::borderControl));
        g.drawRoundedRectangle(bounds, bounds.getHeight() / 2, 1.0f);

        const auto track = juce::Rectangle<float>(static_cast<float>(1 + switchPadding),
                                                  (static_cast<float>(getHeight()) - trackHeight) / 2,
                                                  trackWidth, trackHeight);
        g.setColour(on ? colour(tokens::deckA).withAlpha(0.25f) : colour(tokens::borderControl));
        g.fillRoundedRectangle(track, trackHeight / 2.0f);
        g.setColour(colour(on ? tokens::deckA : tokens::switchOff));
        g.fillEllipse(track.getX() + (on ? 14.0f : 2.0f), track.getY() + 2.0f, 8.0f, 8.0f);

        g.setColour(colour(tokens::textMuted));
        g.setFont(smallFont(600));
        g.drawText(getButtonText(),
                   getLocalBounds().withTrimmedLeft(juce::roundToInt(track.getRight()) + headerGap),
                   juce::Justification::centredLeft, false);
    }
};

// The mockup's tooltip above a toolbar button: text on bg.
class AutoMixPanel::Tooltip : public juce::Component
{
public:
    Tooltip()
    {
        setInterceptsMouseClicks(false, false);
        setAccessible(false);
    }

    void show(const juce::String& text, juce::Rectangle<int> target)
    {
        setTitle(text);
        const auto font = smallFont(600);
        const auto width = textWidth(font, text) + 16;
        const auto height = juce::roundToInt(font.getHeight()) + 8;
        setBounds(target.getCentreX() - width / 2, target.getY() - tooltipGap - height, width,
                  height);
        setVisible(true);
        toFront(false);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(colour(tokens::text));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), static_cast<float>(tokens::radiusSmall));
        g.setColour(colour(tokens::bg));
        g.setFont(smallFont(600));
        g.drawText(getTitle(), getLocalBounds(), juce::Justification::centred, false);
    }
};

// The row's × button, recycled by the list for any row; it removes whatever
// row it currently shows, so it is never replaced while its click runs.
class AutoMixPanel::RowButtons : public juce::Component
{
public:
    explicit RowButtons(AutoMixPanel& panelToUse) : panel(panelToUse)
    {
        setInterceptsMouseClicks(false, true);
        remove.setButtonText(utf8(strings::removeFromQueue));
        remove.getProperties().set(TurbitoLookAndFeel::radiusProperty, tokens::radiusSmall);
        TurbitoLookAndFeel::setButtonFont(remove, tokens::fontSizeBody, 400);
        remove.setWantsKeyboardFocus(false);
        remove.onClick = [this] { panel.removeRow(row); };
        addAndMakeVisible(remove);
    }

    void update(int newRow)
    {
        row = newRow;
        const auto valid = row >= 0 && row < static_cast<int>(panel.queue.size());
        remove.setVisible(valid);
        remove.setEnabled(valid && panel.queue[static_cast<std::size_t>(row)].status
                                       != QueueStatus::Playing);
    }

    void resized() override
    {
        remove.setBounds(getWidth() - rowPadding - removeSize, (getHeight() - 1 - removeSize) / 2,
                         removeSize, removeSize);
    }

    juce::TextButton remove;

private:
    AutoMixPanel& panel;
    int row = -1;
};

//==============================================================================
AutoMixPanel::AutoMixPanel()
    : switchButton(std::make_unique<Switch>()),
      collapseButton(std::make_unique<IconButton>(strings::collapseAutoMix, icon("M9 6l6 6-6 6"),
                                                  13.0f, static_cast<float>(tokens::radiusSmall))),
      tooltip(std::make_unique<Tooltip>())
{
    switchButton->onClick = [this] { setActive(! active); };
    collapseButton->onClick = [this] { setCollapsed(true); };
    addAndMakeVisible(*switchButton);
    addAndMakeVisible(*collapseButton);

    const juce::Path icons[] = { decksIcon(), icon("M3 7h4l10 10h4M3 17h4L17 7h4"),
                                 icon("M16 3h5v5M4 20L21 3M21 16v5h-5M15 15l6 6M4 4l5 5"),
                                 icon("M9 6h12M9 12h12M9 18h12M4 6h.01M4 12h.01M4 18h.01") };

    for (std::size_t i = 0; i < menuButtons.size(); ++i)
    {
        auto& button = menuButtons[i];
        button = std::make_unique<IconButton>(utf8(strings::autoMixMenus[i]), icons[i], 16.0f, 7.0f);
        button->onClick = [this, i] { showMenu(static_cast<int>(i)); };
        button->onStateChange = [this] { updateTooltip(); };
        addAndMakeVisible(*button);
    }

    list.setModel(this);
    list.setRowHeight(rowHeight);
    list.setWantsKeyboardFocus(false);
    // Hidden like macOS overlay scrollbars, so rows keep the mockup's width; the wheel still scrolls.
    list.getViewport()->setScrollBarsShown(false, false, true, false);
    list.setColour(juce::ListBox::outlineColourId, colour(tokens::bg).withAlpha(0.0f));
    addAndMakeVisible(list);

    addChildComponent(*tooltip);
    setActive(true);
    queueChanged();
}

AutoMixPanel::~AutoMixPanel()
{
    list.setModel(nullptr);
}

void AutoMixPanel::setQueue(std::vector<QueueItem> items)
{
    queue = std::move(items);
    queueChanged();
}

void AutoMixPanel::addTrack(const LibraryTrack& track)
{
    const auto orDash = [](const juce::String& s) { return s.isEmpty() ? utf8(strings::noValue) : s; };
    queue.push_back({ orDash(track.title), orDash(track.artist), orDash(track.bpm),
                      orDash(track.key), orDash(track.duration), QueueStatus::Queued });
    queueChanged();
}

void AutoMixPanel::removeRow(int row)
{
    if (row < 0 || row >= static_cast<int>(queue.size())
        || queue[static_cast<std::size_t>(row)].status == QueueStatus::Playing)
        return;

    queue.erase(queue.begin() + static_cast<std::ptrdiff_t>(row));
    queueChanged();
}

void AutoMixPanel::queueChanged()
{
    duplicates = duplicateFlags(queue);
    list.updateContent();
    list.repaint();
    repaint();
}

void AutoMixPanel::setActive(bool shouldBeActive)
{
    active = shouldBeActive;
    switchButton->setToggleState(active, juce::dontSendNotification);
    switchButton->setButtonText(active ? strings::autoMixOn : strings::autoMixOff);
    resized();
    repaint();
}

void AutoMixPanel::setCollapsed(bool shouldCollapse)
{
    if (collapsed == shouldCollapse)
        return;

    collapsed = shouldCollapse;

    for (auto* child : getChildren())
        child->setVisible(! collapsed && child != tooltip.get());

    repaint();

    if (onCollapsedChanged)
        onCollapsedChanged(collapsed);
}

void AutoMixPanel::mouseUp(const juce::MouseEvent& event)
{
    if (collapsed && getLocalBounds().contains(event.getPosition()))
        setCollapsed(false);
}

//==============================================================================
void AutoMixPanel::showMenu(int index)
{
    juce::PopupMenu menu;
    const auto radio = [&menu](int id, const juce::String& text, bool ticked)
    {
        juce::PopupMenu::Item item(text);
        item.setID(id).setTicked(ticked);
        item.shortcutKeyDescription = TurbitoLookAndFeel::radioMark;
        menu.addItem(item);
    };

    switch (index)
    {
        case 0:
            radio(1, utf8(strings::deckModes[0]), ! options.singleDeck);
            radio(2, utf8(strings::deckModes[1]), options.singleDeck);
            break;
        case 1:
        {
            menu.addItem(1, utf8(strings::trimSilence), true, options.trimSilence);
            menu.addItem(2, utf8(strings::matchBpm), true, options.matchBpm);
            menu.addSeparator();
            menu.addSectionHeader(utf8(options.matchBpm ? strings::durationInBeats
                                                        : strings::durationInSeconds));
            const auto& values = options.matchBpm ? durationsInBeats : durationsInSeconds;
            const auto current = options.matchBpm ? options.beats : options.seconds;
            for (int i = 0; i < 3; ++i)
                radio(10 + i, juce::String(values[i]) + (options.matchBpm ? strings::beats
                                                                          : strings::seconds),
                      values[i] == current);
            break;
        }
        case 2:
            menu.addItem(1, utf8(strings::randomOrder), true, options.random);
            menu.addItem(2, utf8(strings::shuffleList));
            break;
        default:
            for (int i = 0; i < 3; ++i)
                menu.addItem(1 + i, utf8(strings::listActions[i]));
            menu.addItem(4, utf8(strings::autoRemovePlayed), true, options.autoRemove);
            menu.addSeparator();
            menu.addItem(5, utf8(strings::saveList));
            menu.addItem(6, utf8(strings::loadList));
            break;
    }

    auto& button = *menuButtons[static_cast<std::size_t>(index)];
    const auto screen = button.getScreenBounds();
    // The first two menus open left-aligned to their button, the others right-aligned.
    const auto x = index < 2 ? screen.getX() : screen.getRight() - menuWidth;

    openMenu = index;
    button.open = true;
    button.repaint();
    updateTooltip();

    menu.showMenuAsync(juce::PopupMenu::Options()
                           .withTargetComponent(button)
                           .withTargetScreenArea({ x, screen.getY(), menuWidth,
                                                   screen.getHeight() + menuOffset })
                           .withMinimumWidth(menuWidth),
                       [safe = juce::Component::SafePointer<AutoMixPanel>(this), index](int result)
                       {
                           if (safe == nullptr)
                               return;

                           safe->openMenu = -1;
                           safe->menuButtons[static_cast<std::size_t>(index)]->open = false;
                           safe->menuButtons[static_cast<std::size_t>(index)]->repaint();

                           if (result != 0)
                               safe->pick(index, result);

                           safe->updateTooltip();
                       });
}

void AutoMixPanel::pick(int menu, int id)
{
    // Revolver, Limpiar, Quitar…, Guardar and Cargar are inert until phase 10.
    switch (menu)
    {
        case 0: options.singleDeck = id == 2; break;
        case 1:
            if (id == 1) options.trimSilence = ! options.trimSilence;
            else if (id == 2) options.matchBpm = ! options.matchBpm;
            else if (options.matchBpm) options.beats = durationsInBeats[id - 10];
            else options.seconds = durationsInSeconds[id - 10];
            break;
        case 2: if (id == 1) options.random = ! options.random; break;
        default: if (id == 4) options.autoRemove = ! options.autoRemove; break;
    }

    list.repaint();  // Siguiente follows the deck mode
    repaint();
}

void AutoMixPanel::updateTooltip()
{
    for (int i = 0; i < static_cast<int>(menuButtons.size()); ++i)
    {
        const auto& button = *menuButtons[static_cast<std::size_t>(i)];

        if (button.isOver() && i != openMenu && ! collapsed)
        {
            tooltip->show(button.getTitle(), button.getBounds());
            return;
        }
    }

    tooltip->setVisible(false);
}

//==============================================================================
void AutoMixPanel::resized()
{
    auto area = getLocalBounds().reduced(inset);

    header = area.removeFromTop(headerHeight);
    auto right = header;
    collapseButton->setBounds(right.removeFromRight(headerHeight));
    right.removeFromRight(headerGap);
    switchButton->setBounds(right.removeFromRight(switchButton->preferredWidth()));
    header.setRight(right.getRight() - rowGap);
    area.removeFromTop(rowGap);

    auto toolbar = area.removeFromTop(menuButtonSize);
    for (auto& button : menuButtons)
    {
        button->setBounds(toolbar.removeFromLeft(menuButtonSize));
        toolbar.removeFromLeft(menuButtonGap);
    }
    summaryArea = toolbar.withTrimmedLeft(rowGap - menuButtonGap);
    area.removeFromTop(rowGap);

    queueFrame = area;
    list.setBounds(area.reduced(1));
}

void AutoMixPanel::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour(colour(tokens::bg));
    g.fillRoundedRectangle(bounds, panelRadius);
    g.setColour(colour(tokens::border));
    g.drawRoundedRectangle(bounds.reduced(0.5f), panelRadius - 0.5f, 1.0f);

    const auto pending = juce::String(pendingCount(queue));

    if (collapsed)
    {
        // Strip: chevron, status dot and the vertical label, centred.
        const auto centreX = bounds.getCentreX();
        auto y = 1.0f + stripPadding;

        g.setColour(colour(tokens::text));
        strokeIcon(g, icon("M15 6l-6 6 6 6"), { centreX - 7.0f, y, 14.0f, 14.0f });
        y += 14.0f + stripGap;

        g.setColour(colour(active ? tokens::deckA : tokens::borderStrong));
        g.fillEllipse(centreX - 4.0f, y, 8.0f, 8.0f);
        y += 8.0f + stripGap;

        // Rotated a quarter turn clockwise, like writing-mode: vertical-rl.
        const auto font = fonts::tracked(fonts::ui(static_cast<float>(tokens::fontSizeButton), 600), 0.06f);
        const auto text = utf8(strings::autoMixStrip) + pending;
        const auto length = static_cast<float>(textWidth(font, text));
        g.setColour(colour(tokens::text));
        g.setFont(font);
        g.addTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::halfPi)
                           .translated(centreX + font.getHeight() / 2, y));
        g.drawText(text, juce::Rectangle<float>(length, font.getHeight()),
                   juce::Justification::centredLeft, false);
        return;
    }

    // Header: title and count share a baseline.
    const auto titleFont = fonts::ui(14.0f, 700);
    const auto countFont = smallFont();
    const auto title = utf8(strings::autoMix);
    const auto baseline = static_cast<float>(header.getCentreY())
                        + (titleFont.getAscent() - titleFont.getDescent()) / 2;
    const auto titleWidth = juce::jmin(textWidth(titleFont, title), header.getWidth());

    g.setColour(colour(tokens::text));
    g.setFont(titleFont);
    g.drawText(title, juce::Rectangle<float>(static_cast<float>(header.getX()),
                                             baseline - titleFont.getAscent(),
                                             static_cast<float>(titleWidth), titleFont.getHeight()),
               juce::Justification::centredLeft, false);

    const auto countX = static_cast<float>(header.getX() + titleWidth + rowGap);
    g.setColour(colour(tokens::textMuted));
    g.setFont(countFont);
    g.drawText(pending + strings::pendingTracks,
               juce::Rectangle<float>(countX, baseline - countFont.getAscent(),
                                      juce::jmax(0.0f, static_cast<float>(header.getRight()) - countX),
                                      countFont.getHeight()),
               juce::Justification::centredLeft, true);

    g.drawText(autoMixSummary(options), summaryArea, juce::Justification::centredLeft, true);

    if (queue.empty())
    {
        const auto font = fonts::ui(static_cast<float>(tokens::fontSizeButton));
        const auto text = queueFrame.reduced(1).reduced(12, 24);
        g.setFont(font);
        g.drawMultiLineText(utf8(strings::emptyQueue), text.getX(),
                            text.getY() + juce::roundToInt(font.getAscent()), text.getWidth(),
                            juce::Justification::centred);
    }
}

void AutoMixPanel::paintOverChildren(juce::Graphics& g)
{
    if (collapsed)
        return;

    // Round the queue's corners over its rows, like overflow with border-radius.
    const auto frame = queueFrame.toFloat();
    juce::Path corners;
    corners.addRectangle(frame);
    corners.addRoundedRectangle(frame, queueRadius);
    corners.setUsingNonZeroWinding(false);
    g.setColour(colour(tokens::bg));
    g.fillPath(corners);

    g.setColour(colour(tokens::border));
    g.drawRoundedRectangle(frame.reduced(0.5f), queueRadius - 0.5f, 1.0f);
}

//==============================================================================
int AutoMixPanel::getNumRows()
{
    return static_cast<int>(queue.size());
}

void AutoMixPanel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool)
{
    if (row < 0 || row >= static_cast<int>(queue.size()))
        return;

    const auto& item = queue[static_cast<std::size_t>(row)];
    const auto style = styleOf(item.status, options.singleDeck);
    const auto inner = height - 1;

    if (item.status == QueueStatus::Playing)
        g.fillAll(colour(tokens::rowActive));

    g.setColour(colour(tokens::empty));
    g.fillRect(0, inner, width, 1);

    // Number | title, badge, Duplicada / meta | duration | ×
    auto area = juce::Rectangle<int>(width, inner).reduced(rowPadding, 0);
    const auto mono = fonts::mono(static_cast<float>(tokens::fontSizeLabel));

    g.setColour(colour(tokens::textMuted));
    g.setFont(mono);
    g.drawText(juce::String(row + 1), area.removeFromLeft(numberWidth),
               juce::Justification::centredLeft, false);
    area.removeFromLeft(rowGap);

    area.removeFromRight(removeSize + rowGap);
    g.drawText(item.duration, area.removeFromRight(textWidth(mono, item.duration)),
               juce::Justification::centredLeft, false);
    area.removeFromRight(rowGap);

    const auto metaFont = smallFont();
    const auto metaHeight = juce::roundToInt(metaFont.getHeight());
    const auto top = (inner - titleLine - metaHeight) / 2;
    auto line = juce::Rectangle<int>(area.getX(), top, area.getWidth(), titleLine);

    g.setColour(colour(tokens::textMuted));
    g.setFont(metaFont);
    const auto separator = utf8(strings::separator);
    g.drawText(item.artist + separator + item.bpm + strings::bpmSuffix + separator + item.key,
               area.withY(top + titleLine).withHeight(metaHeight),
               juce::Justification::centredLeft, true);

    // Badge and Duplicada keep their size; the title shrinks with an ellipsis.
    const auto badgeFont = smallFont(700);
    const auto dupeFont = smallFont(600);
    const auto badgeWidth = textWidth(badgeFont, style.label) + 2 * badgePadding;
    const auto dupe = row < static_cast<int>(duplicates.size()) && duplicates[static_cast<std::size_t>(row)];
    const auto dupeWidth = dupe ? titleGap + textWidth(dupeFont, utf8(strings::duplicate)) : 0;
    const auto titleFont = fonts::ui(static_cast<float>(tokens::fontSizeBody), 500);
    const auto titleWidth = juce::jlimit(0, juce::jmax(0, line.getWidth() - titleGap - badgeWidth - dupeWidth),
                                         textWidth(titleFont, item.title));

    g.setColour(colour(style.text));
    g.setFont(titleFont);
    g.drawText(item.title, line.removeFromLeft(titleWidth), juce::Justification::centredLeft, true);
    line.removeFromLeft(titleGap);

    const auto badge = line.removeFromLeft(badgeWidth);
    g.setColour(colour(style.badge));
    g.fillRoundedRectangle(badge.toFloat(), titleLine / 2.0f);
    g.setColour(colour(style.badgeText));
    g.setFont(badgeFont);
    g.drawText(style.label, badge, juce::Justification::centred, false);

    if (dupe)
    {
        g.setColour(colour(tokens::duplicateWarning));
        g.setFont(dupeFont);
        g.drawText(strings::duplicate, line.withTrimmedLeft(titleGap),
                   juce::Justification::centredLeft, false);
    }
}

juce::Component* AutoMixPanel::refreshComponentForRow(int row, bool, juce::Component* existing)
{
    auto* buttons = dynamic_cast<RowButtons*>(existing);

    if (buttons == nullptr)
    {
        delete existing;
        buttons = new RowButtons(*this);
    }

    buttons->update(row);
    return buttons;
}
