#include "app/MainComponent.h"
#include "ui/look/Tokens.h"

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(tokens::bg));
}
