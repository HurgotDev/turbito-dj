#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Draws every JUCE widget like design/mockup-reference.html. Colours come
// from tokens; per-widget accents go through the usual colour IDs.
class TurbitoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum ColourIds
    {
        // Button border; set it on a button for accent borders.
        buttonBorderColourId = 0x7b00001,
        // Tick and radio marks in popup menus.
        menuMarkColourId = 0x7b00002
    };

    // Button corner radius: button.getProperties().set(radiusProperty, 6).
    static inline const juce::Identifier radiusProperty { "radius" };
    // Button font other than the height rule: setButtonFont(button, size, weight, mono).
    static inline const juce::Identifier fontSizeProperty { "fontSize" };
    static inline const juce::Identifier fontWeightProperty { "fontWeight" };
    static inline const juce::Identifier monoProperty { "mono" };
    // Our menus show no shortcuts, so a ticked item whose
    // shortcutKeyDescription is radioMark draws a radio dot instead of a tick.
    static inline const juce::String radioMark { "radio" };

    TurbitoLookAndFeel();

    // Inter 11 uppercase with 0.08em tracking in textMuted, as the mockup's
    // section labels.
    static void drawLabelText(juce::Graphics&, const juce::String& text,
                              juce::Rectangle<float> area, juce::Justification);

    // Panel chrome: panel fill, 1px border, radius 12.
    static void drawPanel(juce::Graphics&, juce::Rectangle<float> bounds);

    static void setButtonFont(juce::Button&, int size, int weight, bool mono = false);

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                              bool isHighlighted, bool isDown) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool isHighlighted,
                        bool isDown) override;

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float position,
                          float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, float position,
                          float minPosition, float maxPosition, juce::Slider::SliderStyle,
                          juce::Slider&) override;
    int getSliderThumbRadius(juce::Slider&) override;

    void drawComboBox(juce::Graphics&, int width, int height, bool isDown, int buttonX,
                      int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

    void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area, bool isSeparator,
                           bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;
    void drawPopupMenuSectionHeader(juce::Graphics&, const juce::Rectangle<int>& area,
                                    const juce::String& sectionName) override;
    juce::Font getPopupMenuFont() override;
    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
                                   int standardMenuItemHeight, int& idealWidth,
                                   int& idealHeight) override;
    void getIdealPopupMenuSectionHeaderSizeWithOptions(const juce::String& text,
                                                       int standardMenuItemHeight,
                                                       int& idealWidth, int& idealHeight,
                                                       const juce::PopupMenu::Options&) override;
    int getPopupMenuBorderSize() override;
    int getMenuWindowFlags() override;
    void preparePopupMenuWindow(juce::Component&) override;

    void fillTextEditorBackground(juce::Graphics&, int width, int height,
                                  juce::TextEditor&) override;
    void drawTextEditorOutline(juce::Graphics&, int width, int height,
                               juce::TextEditor&) override;

    void drawTableHeaderBackground(juce::Graphics&, juce::TableHeaderComponent&) override;
    void drawTableHeaderColumn(juce::Graphics&, juce::TableHeaderComponent&,
                               const juce::String& columnName, int columnId, int width,
                               int height, bool isMouseOver, bool isMouseDown,
                               int columnFlags) override;

    int getDefaultScrollbarWidth() override;
    void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int x, int y, int width, int height,
                       bool isVertical, int thumbStart, int thumbSize, bool isMouseOver,
                       bool isMouseDown) override;

    juce::Rectangle<int> getTooltipBounds(const juce::String& text, juce::Point<int> screenPos,
                                          juce::Rectangle<int> parentArea) override;
    void drawTooltip(juce::Graphics&, const juce::String& text, int width, int height) override;
};
