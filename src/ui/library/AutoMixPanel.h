#pragma once

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/library/LibraryComponent.h"

enum class QueueStatus { Played, Playing, Next, Queued };

struct QueueItem
{
    juce::String title, artist, bpm, key, duration;
    QueueStatus status = QueueStatus::Queued;
};

// The options of the four Auto Mix menus, with the mockup's defaults.
struct AutoMixOptions
{
    bool singleDeck = false;
    bool trimSilence = true;
    bool matchBpm = true;
    int beats = 16;
    int seconds = 8;
    bool random = false;
    bool autoRemove = false;
};

// "Decks A/B · Sin silencios · BPM emparejado · 16 beats", as the mockup.
juce::String autoMixSummary(const AutoMixOptions&);
// Items not yet played.
int pendingCount(const std::vector<QueueItem>&);
// True for each item whose title already appeared earlier in the queue.
std::vector<bool> duplicateFlags(const std::vector<QueueItem>&);

// Auto Mix side panel: header with switch and collapse, the four option
// menus with their summary, and the queue. Collapses to a 44-wide strip.
// Phase 4 is UI only: nothing loads or mixes until phase 10.
class AutoMixPanel : public juce::Component, private juce::ListBoxModel
{
public:
    AutoMixPanel();
    ~AutoMixPanel() override;

    void setQueue(std::vector<QueueItem>);
    // Appended as En cola; empty fields read "—".
    void addTrack(const LibraryTrack&);
    const std::vector<QueueItem>& getQueue() const noexcept { return queue; }

    bool isActive() const noexcept { return active; }
    const AutoMixOptions& getOptions() const noexcept { return options; }

    bool isCollapsed() const noexcept { return collapsed; }
    void setCollapsed(bool);
    std::function<void(bool collapsed)> onCollapsedChanged;

    void resized() override;
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    class IconButton;
    class Switch;
    class Tooltip;
    class RowButtons;

    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int width, int height,
                          bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int row, bool isRowSelected,
                                            juce::Component* existing) override;

    void queueChanged();
    void removeRow(int row);
    void setActive(bool);
    void showMenu(int index);
    void pick(int menu, int id);
    void updateTooltip();

    std::vector<QueueItem> queue;
    std::vector<bool> duplicates;
    AutoMixOptions options;
    bool active = true;
    bool collapsed = false;
    int openMenu = -1;

    std::unique_ptr<Switch> switchButton;
    std::unique_ptr<IconButton> collapseButton;
    std::array<std::unique_ptr<IconButton>, 4> menuButtons;
    std::unique_ptr<Tooltip> tooltip;
    juce::ListBox list;

    juce::Rectangle<int> header, summaryArea, queueFrame;
};
