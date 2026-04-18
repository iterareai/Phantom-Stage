#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout XTCProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Speaker spread (total angle between speakers, as seen from listening position)
    // e.g. 40 degrees = left speaker 20° left, right speaker 20° right
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "angle", "Speaker Spread",
        juce::NormalisableRange<float>(20.0f, 120.0f, 1.0f), 40.0f,
        juce::AudioParameterFloatAttributes().withLabel("deg")));

    // Cancellation amount (attenuation of cross-feed signal)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "cancel", "Cancellation",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.6f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Bass boost (low shelf at 70Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "bassboost", "Bass Boost",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // High-frequency shelf gain (XTC works best with slight HF boost)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hfboost", "HF Boost",
        juce::NormalisableRange<float>(-6.0f, 12.0f, 0.1f), 3.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Output gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Output Gain",
        juce::NormalisableRange<float>(-12.0f, 6.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "bypass", "Bypass", false));

    return { params.begin(), params.end() };
}

XTCProcessor::XTCProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    memset(delayBufferL, 0, sizeof(delayBufferL));
    memset(delayBufferR, 0, sizeof(delayBufferR));
    memset(delayWritePos, 0, sizeof(delayWritePos));
}

XTCProcessor::~XTCProcessor() {}

//==============================================================================
void XTCProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    memset(delayBufferL, 0, sizeof(delayBufferL));
    memset(delayBufferR, 0, sizeof(delayBufferR));
    memset(delayWritePos, 0, sizeof(delayWritePos));
    bassShelfL = {};
    bassShelfR = {};
    updateBassShelfCoeffs(0.0f, 70.0f);
}

void XTCProcessor::releaseResources() {}

//==============================================================================
void XTCProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    auto* bypassParam  = apvts.getRawParameterValue("bypass");
    if (bypassParam->load() > 0.5f) return;

    auto* angleParam    = apvts.getRawParameterValue("angle");
    auto* cancelParam   = apvts.getRawParameterValue("cancel");
    auto* hfboostParam  = apvts.getRawParameterValue("hfboost");
    auto* gainParam     = apvts.getRawParameterValue("gain");
    auto* bassBoostParam = apvts.getRawParameterValue("bassboost");

    const float angleDeg   = angleParam->load();
    const float cancel     = cancelParam->load();
    const float hfBoostDB  = hfboostParam->load();
    const float gainLinear = juce::Decibels::decibelsToGain(gainParam->load());
    const float bassBoostDB = bassBoostParam->load();

    // Update bass shelf coefficients
    updateBassShelfCoeffs(bassBoostDB, 70.0f);

    // Calculate inter-aural delay based on speaker spread
    // angleDeg is the TOTAL angle between speakers (e.g. 40° = ±20° from centre)
    // We use half the spread to get the angle from centre to one speaker
    const float speedOfSound = 343.0f;
    const float headRadius   = 0.0875f; // metres
    const float halfAngleRad = (angleDeg * 0.5f) * juce::MathConstants<float>::pi / 180.0f;
    const float delaySeconds = (headRadius / speedOfSound) * std::sin(halfAngleRad);

    // Each successive stage doubles the delay
    // Stage n delay = delaySeconds * (2^n) * cancel^n attenuation
    float stageDelaySamples[NUM_STAGES];
    float stageGain[NUM_STAGES];
    float baseDelay = (float)(delaySeconds * currentSampleRate);
    for (int s = 0; s < NUM_STAGES; ++s)
    {
        stageDelaySamples[s] = baseDelay * (float)(1 << s); // 1x, 2x, 4x, 8x
        stageGain[s] = std::pow(cancel, (float)(s + 1));    // cancel^1, ^2, ^3, ^4
    }

    // Simple 1-pole HF shelf for the cross-feed path
    // cutoff ~ 2kHz
    const float hfLinear = juce::Decibels::decibelsToGain(hfBoostDB);
    const float shelfFreq = 2000.0f;
    const float shelfCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * shelfFreq / (float)currentSampleRate);
    static float hfStateL = 0.0f, hfStateR = 0.0f;

    const int numSamples = buffer.getNumSamples();
    auto* dataL = buffer.getWritePointer(0);
    auto* dataR = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = dataL[i];
        float inR = dataR[i];

        float xfeedL = 0.0f; // cross-feed injected into left (from right)
        float xfeedR = 0.0f; // cross-feed injected into right (from left)

        for (int s = 0; s < NUM_STAGES; ++s)
        {
            int delaySamp = juce::jmin((int)stageDelaySamples[s], MAX_DELAY_SAMPLES - 1);
            delaySamp = juce::jmax(delaySamp, 1);

            int readPos = (delayWritePos[s] - delaySamp + MAX_DELAY_SAMPLES) % MAX_DELAY_SAMPLES;

            // Read delayed cross-channel samples
            float delayedR = delayBufferR[s][readPos];
            float delayedL = delayBufferL[s][readPos];

            // Apply HF shelf to cross-feed (warm it slightly)
            float filteredR = delayedR + shelfCoeff * (hfStateR - delayedR) * (hfLinear - 1.0f);
            float filteredL = delayedL + shelfCoeff * (hfStateL - delayedL) * (hfLinear - 1.0f);
            hfStateR = filteredR;
            hfStateL = filteredL;

            // XTC: inject inverted, attenuated cross-feed
            xfeedL -= stageGain[s] * filteredR;
            xfeedR -= stageGain[s] * filteredL;

            // Write current input into delay buffers
            delayBufferL[s][delayWritePos[s]] = inL;
            delayBufferR[s][delayWritePos[s]] = inR;
            delayWritePos[s] = (delayWritePos[s] + 1) % MAX_DELAY_SAMPLES;
        }

        float outL = (inL + xfeedL) * gainLinear;
        float outR = (inR + xfeedR) * gainLinear;

        // Apply bass shelf boost
        dataL[i] = processBiquad(outL, bassShelfL, bassCoeffs);
        dataR[i] = processBiquad(outR, bassShelfR, bassCoeffs);
    }
}

//==============================================================================
juce::AudioProcessorEditor* XTCProcessor::createEditor()
{
    return new XTCEditor (*this);
}

void XTCProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void XTCProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
void XTCProcessor::updateBassShelfCoeffs(float gainDB, float shelfFreqHz)
{
    // 2nd order low shelf filter (Audio EQ Cookbook)
    const float A  = std::pow(10.0f, gainDB / 40.0f);
    const float w0 = 2.0f * juce::MathConstants<float>::pi * shelfFreqHz / (float)currentSampleRate;
    const float cosw0 = std::cos(w0);
    const float S  = 1.0f; // shelf slope
    const float alpha = std::sin(w0) / 2.0f * std::sqrt((A + 1.0f/A) * (1.0f/S - 1.0f) + 2.0f);

    const float b0 =    A * ((A+1) - (A-1)*cosw0 + 2*std::sqrt(A)*alpha);
    const float b1 =  2*A * ((A-1) - (A+1)*cosw0);
    const float b2 =    A * ((A+1) - (A-1)*cosw0 - 2*std::sqrt(A)*alpha);
    const float a0 =        (A+1) + (A-1)*cosw0 + 2*std::sqrt(A)*alpha;
    const float a1 =   -2 * ((A-1) + (A+1)*cosw0);
    const float a2 =        (A+1) + (A-1)*cosw0 - 2*std::sqrt(A)*alpha;

    bassCoeffs.b0 = b0 / a0;
    bassCoeffs.b1 = b1 / a0;
    bassCoeffs.b2 = b2 / a0;
    bassCoeffs.a1 = a1 / a0;
    bassCoeffs.a2 = a2 / a0;
}

float XTCProcessor::processBiquad(float x, BiquadState& s, const BiquadCoeffs& c)
{
    float y = c.b0*x + c.b1*s.x1 + c.b2*s.x2 - c.a1*s.y1 - c.a2*s.y2;
    s.x2 = s.x1; s.x1 = x;
    s.y2 = s.y1; s.y1 = y;
    return y;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XTCProcessor();
}
