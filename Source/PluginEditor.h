#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class XTCEditor : public juce::AudioProcessorEditor
{
public:
    XTCEditor (XTCProcessor&);
    ~XTCEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    XTCProcessor& processorRef;

    // Knobs
    juce::Slider angleKnob, cancelKnob, hfBoostKnob, gainKnob, bassBoostKnob;
    juce::Label  angleLabel, cancelLabel, hfBoostLabel, gainLabel, bassBoostLabel;
    juce::ToggleButton bypassButton;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> angleAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cancelAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hfBoostAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassBoostAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    void setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XTCEditor)
};
