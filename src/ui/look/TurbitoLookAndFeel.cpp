#include "ui/look/TurbitoLookAndFeel.h"

#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"

namespace
{
    juce::Colour colour(std::uint32_t argb)
    {
        return juce::Colour(argb);
    }

    // Mockup menu geometry: 6 padding inside a 1px border, 32-high items
    // with 2px gaps, so each item slot is 34 with its highlight inset by 1.
    constexpr int menuBorder = 6;
    constexpr int menuItemHeight = 34;
    constexpr int menuSeparatorHeight = 11;
    constexpr int menuHeaderHeight = 23;
    constexpr int menuMarkWidth = 14;
    constexpr int menuTextIndent = 8 + menuMarkWidth + 8;

    constexpr float sliderTrack = 8.0f;
    constexpr int sliderThumbRadius = 8;

    juce::Font tooltipFont()
    {
        return fonts::ui(static_cast<float>(tokens::fontSizeLabel), 600);
    }

    // Lives in the popup window's properties, so it dies with the window.
    struct MenuShadow : juce::ReferenceCountedObject
    {
        juce::DropShadower shadower { juce::DropShadow(juce::Colours::black.withAlpha(0.45f), 24,
                                                       { 0, 8 }) };
    };
}

TurbitoLookAndFeel::TurbitoLookAndFeel()
    : juce::LookAndFeel_V4({ colour(tokens::bg), colour(tokens::surface), colour(tokens::surface),
                             colour(tokens::borderControl), colour(tokens::text),
                             colour(tokens::deckA), colour(tokens::bg), colour(tokens::deckA),
                             colour(tokens::text) })
{
    const auto transparent = colour(tokens::surface).withAlpha(0.0f);

    setColour(buttonBorderColourId, colour(tokens::borderControl));
    setColour(menuMarkColourId, colour(tokens::deckA));

    setColour(juce::TextButton::buttonColourId, colour(tokens::surface));
    setColour(juce::TextButton::buttonOnColourId, colour(tokens::deckA));
    setColour(juce::TextButton::textColourOffId, colour(tokens::text));
    setColour(juce::TextButton::textColourOnId, colour(tokens::bg));

    setColour(juce::Slider::rotarySliderFillColourId, colour(tokens::deckA));
    setColour(juce::Slider::rotarySliderOutlineColourId, colour(tokens::borderControl));
    setColour(juce::Slider::trackColourId, colour(tokens::deckA));
    setColour(juce::Slider::thumbColourId, colour(tokens::deckA));
    setColour(juce::Slider::backgroundColourId, colour(tokens::borderStrong));

    setColour(juce::ComboBox::backgroundColourId, colour(tokens::surface));
    setColour(juce::ComboBox::outlineColourId, colour(tokens::borderControl));
    setColour(juce::ComboBox::textColourId, colour(tokens::text));
    setColour(juce::ComboBox::arrowColourId, colour(tokens::text));

    // Transparent so the menu window is not opaque and its corners can round.
    setColour(juce::PopupMenu::backgroundColourId, transparent);
    setColour(juce::PopupMenu::textColourId, colour(tokens::text));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, colour(tokens::surfaceHover));
    setColour(juce::PopupMenu::highlightedTextColourId, colour(tokens::text));

    setColour(juce::TextEditor::backgroundColourId, colour(tokens::bg));
    setColour(juce::TextEditor::textColourId, colour(tokens::text));
    setColour(juce::TextEditor::outlineColourId, colour(tokens::borderControl));
    setColour(juce::TextEditor::focusedOutlineColourId, colour(tokens::borderControl));
    setColour(juce::TextEditor::highlightColourId, colour(tokens::deckA).withAlpha(0.3f));
    setColour(juce::CaretComponent::caretColourId, colour(tokens::text));

    setColour(juce::TableHeaderComponent::backgroundColourId, colour(tokens::surface));
    setColour(juce::TableHeaderComponent::textColourId, colour(tokens::textMuted));
    setColour(juce::ListBox::backgroundColourId, transparent);

    setColour(juce::ScrollBar::thumbColourId, colour(tokens::borderControl));
    setColour(juce::ScrollBar::trackColourId, transparent);

    setColour(juce::TooltipWindow::backgroundColourId, colour(tokens::text));
    setColour(juce::TooltipWindow::textColourId, colour(tokens::bg));

    setColour(juce::Label::textColourId, colour(tokens::text));
}

void TurbitoLookAndFeel::drawLabelText(juce::Graphics& g, const juce::String& text,
                                       juce::Rectangle<float> area, juce::Justification justification)
{
    g.setColour(colour(tokens::textMuted));
    g.setFont(fonts::tracked(fonts::ui(static_cast<float>(tokens::fontSizeLabel)),
                             tokens::labelTracking));
    g.drawText(text.toUpperCase(), area, justification, false);
}

void TurbitoLookAndFeel::drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const auto radius = static_cast<float>(tokens::radiusPanel);

    g.setColour(colour(tokens::panel));
    g.fillRoundedRectangle(bounds, radius);
    g.setColour(colour(tokens::border));
    g.drawRoundedRectangle(bounds.reduced(0.5f), radius - 0.5f, 1.0f);
}

void TurbitoLookAndFeel::setButtonFont(juce::Button& button, int size, int weight, bool mono)
{
    auto& properties = button.getProperties();
    properties.set(fontSizeProperty, size);
    properties.set(fontWeightProperty, weight);
    properties.set(monoProperty, mono);
}

juce::Typeface::Ptr TurbitoLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    if (font.getTypefaceName() == juce::Font::getDefaultSansSerifFontName())
        return fonts::uiTypeface(font.isBold() ? 700 : 400);

    return juce::LookAndFeel_V4::getTypefaceForFont(font);
}

//==============================================================================
juce::Font TurbitoLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight)
{
    const auto& properties = button.getProperties();

    if (properties.contains(fontSizeProperty))
    {
        const auto size = static_cast<float>(static_cast<int>(properties[fontSizeProperty]));
        const auto weight = static_cast<int>(properties[fontWeightProperty]);
        return properties[monoProperty] ? fonts::mono(size, weight) : fonts::ui(size, weight);
    }

    // The mockup's 40-high buttons (CUE, PLAY) are 13/700; the rest 12/600.
    return buttonHeight >= 40 ? fonts::ui(static_cast<float>(tokens::fontSizeBody), 700)
                              : fonts::ui(static_cast<float>(tokens::fontSizeButton), 600);
}

void TurbitoLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool isHighlighted, bool isDown)
{
    const auto radius = static_cast<float>(
        static_cast<int>(button.getProperties().getWithDefault(radiusProperty, tokens::radiusButton)));
    const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);

    auto fill = backgroundColour;

    if ((isHighlighted || isDown) && ! button.getToggleState() && fill == colour(tokens::surface))
        fill = colour(tokens::surfaceHover);

    if (! button.isEnabled())
        fill = fill.withMultipliedAlpha(0.5f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, radius);

    g.setColour(button.findColour(buttonBorderColourId));
    g.drawRoundedRectangle(bounds, radius, 1.0f);
}

void TurbitoLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool)
{
    const auto colourId = button.getToggleState() ? juce::TextButton::textColourOnId
                                                  : juce::TextButton::textColourOffId;

    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.setColour(button.findColour(colourId).withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));
    g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(4, 0),
                     juce::Justification::centred, 1);
}

//==============================================================================
void TurbitoLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float position, float startAngle, float endAngle,
                                          juce::Slider& slider)
{
    const auto diameter = static_cast<float>(juce::jmin(width, height));
    const auto knob = juce::Rectangle<float>(diameter, diameter)
                          .withCentre(juce::Rectangle<int>(x, y, width, height).toFloat().getCentre());

    g.setColour(colour(tokens::bg));
    g.fillEllipse(knob);

    g.setColour(colour(tokens::borderControl));
    g.drawEllipse(knob.reduced(1.0f), 2.0f);

    // 2px wide, 2px inside the 2px border; 8 long at 26, 7 at 24.
    const auto length = std::round(diameter * 0.3f);
    const juce::Rectangle<float> marker(knob.getCentreX() - 1.0f, knob.getY() + 4.0f, 2.0f, length);
    const auto angle = startAngle + position * (endAngle - startAngle);

    juce::Path path;
    path.addRoundedRectangle(marker, 1.0f);

    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.fillPath(path, juce::AffineTransform::rotation(angle, knob.getCentreX(), knob.getCentreY()));
}

void TurbitoLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float position, float minPosition, float maxPosition,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearVertical)
    {
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, position, minPosition,
                                               maxPosition, style, slider);
        return;
    }

    // Like Chrome's <input type=range> with accent-color: the track spans the
    // whole slider and the thumb centre travels inset by its radius.
    const auto r = static_cast<float>(sliderThumbRadius);
    const auto area = juce::Rectangle<int>(x, y, width, height).toFloat();
    const auto horizontal = slider.isHorizontal();

    const auto track = horizontal
        ? juce::Rectangle<float>(area.getX() - r, area.getCentreY() - sliderTrack / 2,
                                 area.getWidth() + 2 * r, sliderTrack)
        : juce::Rectangle<float>(area.getCentreX() - sliderTrack / 2, area.getY() - r,
                                 sliderTrack, area.getHeight() + 2 * r);

    const auto filled = horizontal ? track.withRight(position) : track.withTop(position);
    const auto thumbCentre = horizontal ? juce::Point<float>(position, track.getCentreY())
                                        : juce::Point<float>(track.getCentreX(), position);
    const auto accent = slider.findColour(juce::Slider::trackColourId);
    const auto corner = sliderTrack / 2;

    g.setColour(slider.findColour(juce::Slider::backgroundColourId));
    g.fillRoundedRectangle(track, corner);
    g.setColour(colour(tokens::switchOff));
    g.drawRoundedRectangle(track.reduced(0.5f), corner - 0.5f, 1.0f);

    g.setColour(accent);
    g.fillRoundedRectangle(filled, corner);
    g.fillEllipse(juce::Rectangle<float>(2 * r, 2 * r).withCentre(thumbCentre));
}

int TurbitoLookAndFeel::getSliderThumbRadius(juce::Slider& slider)
{
    if (slider.getSliderStyle() == juce::Slider::LinearHorizontal
        || slider.getSliderStyle() == juce::Slider::LinearVertical)
        return sliderThumbRadius;

    return juce::LookAndFeel_V4::getSliderThumbRadius(slider);
}

//==============================================================================
void TurbitoLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool, int, int,
                                      int, int, juce::ComboBox& box)
{
    const auto radius = static_cast<float>(height >= 32 ? tokens::radiusButton : tokens::radiusSmall);
    const auto bounds = juce::Rectangle<int>(width, height).toFloat().reduced(0.5f);

    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, radius);
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, radius, 1.0f);

    // Chrome's chevron: 8 × 4, about 7 from the right edge.
    const auto centre = juce::Point<float>(static_cast<float>(width) - 7.0f,
                                           static_cast<float>(height) / 2);
    juce::Path chevron;
    chevron.startNewSubPath(centre.translated(-4.0f, -2.0f));
    chevron.lineTo(centre.translated(0.0f, 2.0f));
    chevron.lineTo(centre.translated(4.0f, -2.0f));

    g.setColour(box.findColour(juce::ComboBox::arrowColourId)
                    .withMultipliedAlpha(box.isEnabled() ? 1.0f : 0.5f));
    g.strokePath(chevron, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

juce::Font TurbitoLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return fonts::ui(static_cast<float>(tokens::fontSizeButton));
}

void TurbitoLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    const auto padding = box.getHeight() >= 32 ? 8 : 6;

    label.setBounds(padding, 0, box.getWidth() - padding - 16, box.getHeight());
    label.setBorderSize({});
    label.setFont(getComboBoxFont(box));
}

//==============================================================================
void TurbitoLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    const auto bounds = juce::Rectangle<int>(width, height).toFloat().reduced(0.5f);
    const auto radius = 10.0f;  // the mockup's menu radius, not a token

    g.setColour(colour(tokens::surface));
    g.fillRoundedRectangle(bounds, radius);
    g.setColour(colour(tokens::borderControl));
    g.drawRoundedRectangle(bounds, radius, 1.0f);
}

void TurbitoLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                           bool isSeparator, bool isActive, bool isHighlighted,
                                           bool isTicked, bool, const juce::String& text,
                                           const juce::String& shortcutKeyText,
                                           const juce::Drawable*, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        g.setColour(colour(tokens::borderControl));
        g.fillRect(area.getX() + 3, area.getY() + 5, area.getWidth() - 6, 1);
        return;
    }

    const auto item = area.reduced(1).toFloat();

    if (isHighlighted && isActive)
    {
        g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
        g.fillRoundedRectangle(item, static_cast<float>(tokens::radiusSmall));
    }

    if (isTicked)
    {
        const auto mark = juce::Rectangle<float>(item.getX() + 8, item.getY(),
                                                 static_cast<float>(menuMarkWidth), item.getHeight());
        g.setColour(findColour(menuMarkColourId));

        if (shortcutKeyText == radioMark)
        {
            g.fillEllipse(juce::Rectangle<float>(5.0f, 5.0f).withCentre(mark.getCentre()));
        }
        else
        {
            const auto box = juce::Rectangle<float>(10.0f, 10.0f).withCentre(mark.getCentre());
            const auto tick = getTickShape(1.0f);
            g.fillPath(tick, tick.getTransformToScaleToFit(box, true));
        }
    }

    auto colourOfText = textColour != nullptr ? *textColour
                                              : findColour(juce::PopupMenu::textColourId);
    g.setColour(colourOfText.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
    g.setFont(getPopupMenuFont());
    g.drawText(text, item.withTrimmedLeft(static_cast<float>(menuTextIndent)).withTrimmedRight(8.0f),
               juce::Justification::centredLeft, true);
}

void TurbitoLookAndFeel::drawPopupMenuSectionHeader(juce::Graphics& g,
                                                    const juce::Rectangle<int>& area,
                                                    const juce::String& sectionName)
{
    // Mockup header padding: 6 top, 8 sides, 2 bottom.
    drawLabelText(g, sectionName, area.reduced(1).toFloat().withTrimmedTop(6.0f)
                                      .withTrimmedBottom(2.0f).reduced(8.0f, 0.0f),
                  juce::Justification::centredLeft);
}

juce::Font TurbitoLookAndFeel::getPopupMenuFont()
{
    return fonts::ui(static_cast<float>(tokens::fontSizeBody));
}

void TurbitoLookAndFeel::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int,
                                                   int& idealWidth, int& idealHeight)
{
    idealHeight = isSeparator ? menuSeparatorHeight : menuItemHeight;
    idealWidth = juce::roundToInt(juce::GlyphArrangement::getStringWidth(getPopupMenuFont(), text))
               + menuTextIndent + 8 + 2;
}

void TurbitoLookAndFeel::getIdealPopupMenuSectionHeaderSizeWithOptions(
    const juce::String& text, int standardMenuItemHeight, int& idealWidth, int& idealHeight,
    const juce::PopupMenu::Options&)
{
    getIdealPopupMenuItemSize(text, false, standardMenuItemHeight, idealWidth, idealHeight);
    idealHeight = menuHeaderHeight;
}

int TurbitoLookAndFeel::getPopupMenuBorderSize()
{
    return menuBorder;
}

int TurbitoLookAndFeel::getMenuWindowFlags()
{
    return 0;  // the shadow comes from preparePopupMenuWindow
}

void TurbitoLookAndFeel::preparePopupMenuWindow(juce::Component& window)
{
    auto* shadow = new MenuShadow();
    shadow->shadower.setOwner(&window);
    window.getProperties().set("turbitoMenuShadow", juce::var(shadow));
}

//==============================================================================
void TurbitoLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                                  juce::TextEditor& editor)
{
    g.setColour(editor.findColour(juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle(juce::Rectangle<int>(width, height).toFloat().reduced(0.5f),
                           static_cast<float>(tokens::radiusButton));
}

void TurbitoLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height,
                                               juce::TextEditor& editor)
{
    g.setColour(editor.findColour(juce::TextEditor::outlineColourId));
    g.drawRoundedRectangle(juce::Rectangle<int>(width, height).toFloat().reduced(0.5f),
                           static_cast<float>(tokens::radiusButton), 1.0f);
}

//==============================================================================
void TurbitoLookAndFeel::drawTableHeaderBackground(juce::Graphics& g,
                                                   juce::TableHeaderComponent& header)
{
    g.fillAll(header.findColour(juce::TableHeaderComponent::backgroundColourId));
}

void TurbitoLookAndFeel::drawTableHeaderColumn(juce::Graphics& g, juce::TableHeaderComponent& header,
                                               const juce::String& columnName, int, int width,
                                               int height, bool, bool, int)
{
    g.setColour(header.findColour(juce::TableHeaderComponent::textColourId));
    g.setFont(fonts::tracked(fonts::ui(static_cast<float>(tokens::fontSizeLabel), 600), 0.06f));
    g.drawText(columnName.toUpperCase(), juce::Rectangle<int>(width, height).reduced(10, 0),
               juce::Justification::centredLeft, true);
}

//==============================================================================
int TurbitoLookAndFeel::getDefaultScrollbarWidth()
{
    return 8;
}

void TurbitoLookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar& bar, int x, int y,
                                       int width, int height, bool isVertical, int thumbStart,
                                       int thumbSize, bool, bool)
{
    const auto thumb = (isVertical ? juce::Rectangle<int>(x, thumbStart, width, thumbSize)
                                   : juce::Rectangle<int>(thumbStart, y, thumbSize, height))
                           .toFloat()
                           .reduced(1.0f);

    g.setColour(bar.findColour(juce::ScrollBar::thumbColourId));
    g.fillRoundedRectangle(thumb, juce::jmin(thumb.getWidth(), thumb.getHeight()) / 2);
}

//==============================================================================
juce::Rectangle<int> TurbitoLookAndFeel::getTooltipBounds(const juce::String& text,
                                                          juce::Point<int> screenPos,
                                                          juce::Rectangle<int> parentArea)
{
    const auto font = tooltipFont();
    const auto w = juce::roundToInt(juce::GlyphArrangement::getStringWidth(font, text)) + 16;
    const auto h = juce::roundToInt(font.getHeight()) + 8;

    return juce::Rectangle<int>(screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12)
                                                                      : screenPos.x + 24,
                                screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)
                                                                      : screenPos.y + 6,
                                w, h)
        .constrainedWithin(parentArea);
}

void TurbitoLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int width,
                                     int height)
{
    // TooltipWindow is always opaque, so the rounded corners show the app
    // background rather than garbage.
    g.fillAll(colour(tokens::bg));
    g.setColour(findColour(juce::TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle(juce::Rectangle<int>(width, height).toFloat(),
                           static_cast<float>(tokens::radiusSmall));

    g.setColour(findColour(juce::TooltipWindow::textColourId));
    g.setFont(tooltipFont());
    g.drawText(text, juce::Rectangle<int>(width, height).reduced(8, 4),
               juce::Justification::centred, true);
}
