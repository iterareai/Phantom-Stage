#pragma once
#include <JuceHeader.h>

//==============================================================================
class XTCProcessor : public juce::AudioProcessor
{
public:
    XTCProcessor();
    ~XTCProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Phantom Stage"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // XTC state: 4 stages of delay lines per channel
    static constexpr int NUM_STAGES = 4;
    static constexpr int MAX_DELAY_SAMPLES = 4096;

    // Circular delay buffers [stage][channel]
    float delayBufferL[NUM_STAGES][MAX_DELAY_SAMPLES] = {};
    float delayBufferR[NUM_STAGES][MAX_DELAY_SAMPLES] = {};
    int   delayWritePos[NUM_STAGES] = {};

    double currentSampleRate = 44100.0;

    // Low-shelf filter state for bass boost (per channel)
    // Using a 2nd order IIR biquad low shelf
    struct BiquadState {
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    };
    BiquadState bassShelfL, bassShelfR;

    struct BiquadCoeffs {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    };
    BiquadCoeffs bassCoeffs;

    void updateBassShelfCoeffs (float gainDB, float shelfFreqHz);
    float processBiquad (float x, BiquadState& s, const BiquadCoeffs& c);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XTCProcessor)
};
