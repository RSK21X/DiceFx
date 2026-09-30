#include "RandomizeHero.h"

RandomizeHero::RandomizeHero (DiceFXAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (randomizeButton);
    addAndMakeVisible (amountSlider);
    randomizeButton.setComponentID ("hero-randomize");
    randomizeButton.setTooltip ("Randomize unlocked parameters.");
    randomizeButton.onClick = [this] { if (onRandomize) onRandomize(); };
    amountSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    amountSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    amountSlider.setScrollWheelEnabled (false);
    amountSlider.setTooltip ("Randomization amount: subtle to wild.");
    amountSlider.onValueChange = [this] { repaint(); };
    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.getAPVTS(), "random_amount", amountSlider);
}

void RandomizeHero::paint (juce::Graphics& g)
{
    DiceTheme::drawPanel (g, getLocalBounds().toFloat(), DiceTheme::background, 4);
    DiceTheme::drawLabel (g, "Amount", { 140, 0, 46, getHeight() }, 11);
    DiceTheme::drawLabel (g, juce::String (juce::roundToInt ((float) amountSlider.getValue() * 100)) + "%",
                          { getWidth() - 43, 0, 37, getHeight() }, 11, DiceTheme::textPrimary);
    if (pulse > 0)
    {
        g.setColour (DiceTheme::accent.withAlpha (pulse * 0.6f));
        g.drawRoundedRectangle (randomizeButton.getBounds().toFloat(), 3, 1);
    }
}

void RandomizeHero::resized()
{
    randomizeButton.setBounds (8, 8, 122, 28);
    amountSlider.setBounds (190, 10, getWidth() - 238, 24);
}

void RandomizeHero::setHistoryAvailability (bool, bool) {}
void RandomizeHero::flashRandomize() { pulse = 1; startTimerHz (30); repaint(); }
void RandomizeHero::timerCallback()
{
    pulse = juce::jmax (0.0f, pulse - 0.08f);
    if (pulse == 0) stopTimer();
    repaint();
}
