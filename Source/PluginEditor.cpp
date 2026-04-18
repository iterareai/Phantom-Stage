#include "PluginEditor.h"

//==============================================================================
XTCEditor::XTCEditor (XTCProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (580, 220);
    setResizable(false, false);

    auto& apvts = processorRef.apvts;

    setupKnob(angleKnob,    angleLabel,    "Spkr Spread");
    setupKnob(cancelKnob,   cancelLabel,   "Cancellation");
    setupKnob(hfBoostKnob,  hfBoostLabel,  "HF Boost");
    setupKnob(bassBoostKnob,bassBoostLabel,"Bass Boost");
    setupKnob(gainKnob,     gainLabel,     "Output Gain");

    angleAttach    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "angle",     angleKnob);
    cancelAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "cancel",    cancelKnob);
    hfBoostAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "hfboost",   hfBoostKnob);
    bassBoostAttach= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "bassboost", bassBoostKnob);
    gainAttach     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "gain",      gainKnob);

    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(juce::ToggleButton::textColourId,   juce::Colours::white);
    bypassButton.setColour(juce::ToggleButton::tickColourId,   juce::Colour(0xff00d4aa));
    bypassButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
    addAndMakeVisible(bypassButton);
    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "bypass", bypassButton);
}

XTCEditor::~XTCEditor() {}

//==============================================================================
void XTCEditor::setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& labelText)
{
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    knob.setColour(juce::Slider::rotarySliderFillColourId,  juce::Colour(0xff00d4aa));
    knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a2a3a));
    knob.setColour(juce::Slider::thumbColourId,             juce::Colours::white);
    knob.setColour(juce::Slider::textBoxTextColourId,       juce::Colours::white);
    knob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff1a1a2a));
    knob.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    addAndMakeVisible(knob);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(12.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xffaaaacc));
    addAndMakeVisible(label);
}

void XTCEditor::paint (juce::Graphics& g)
{
    // Background gradient
    g.fillAll(juce::Colour(0xff12121e));

    // Top banner
    juce::ColourGradient banner(juce::Colour(0xff1e1e35), 0, 0,
                                juce::Colour(0xff0d1225), (float)getWidth(), 50, false);
    g.setGradientFill(banner);
    g.fillRect(0, 0, getWidth(), 50);

    // Accent line under banner
    g.setColour(juce::Colour(0xff00d4aa));
    g.fillRect(0, 49, getWidth(), 2);

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("Phantom Stage", 20, 0, 300, 50, juce::Justification::centredLeft);

    // Subtitle
    g.setFont(juce::Font(11.0f));
    g.setColour(juce::Colour(0xff00d4aa));
    g.drawText("4-Stage Crosstalk Cancellation  |  Spread angle from listening position  |  AU", 20, 28, 480, 20, juce::Justification::centredLeft);

    // Knob panel background
    g.setColour(juce::Colour(0xff1a1a2e));
    g.fillRoundedRectangle(10.0f, 58.0f, (float)getWidth() - 20.0f, 140.0f, 8.0f);
}

void XTCEditor::resized()
{
    const int knobW    = 95;
    const int knobH    = 100;
    const int labelH   = 18;
    const int startY   = 62;
    const int startX   = 20;
    const int spacing  = 108;

    // 5 knobs side by side
    for (int i = 0; i < 5; ++i)
    {
        int x = startX + i * spacing;
        juce::Slider* knob = nullptr;
        juce::Label*  lbl  = nullptr;
        switch (i) {
            case 0: knob = &angleKnob;    lbl = &angleLabel;    break;
            case 1: knob = &cancelKnob;   lbl = &cancelLabel;   break;
            case 2: knob = &hfBoostKnob;  lbl = &hfBoostLabel;  break;
            case 3: knob = &bassBoostKnob;lbl = &bassBoostLabel; break;
            case 4: knob = &gainKnob;     lbl = &gainLabel;     break;
        }
        lbl->setBounds(x, startY, knobW, labelH);
        knob->setBounds(x, startY + labelH, knobW, knobH);
    }

    bypassButton.setBounds(getWidth() - 100, startY + labelH + knobH + 6, 90, 24);
}
