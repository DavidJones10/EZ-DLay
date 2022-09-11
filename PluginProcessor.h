/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
using namespace juce;
class EZDLayAudioProcessor  : public juce::AudioProcessor,
                                public juce::dsp::DelayLine<float>
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    EZDLayAudioProcessor();
    ~EZDLayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================
    void push (float valueToAdd);
    dsp::FirstOrderTPTFilter<double> smoothFilter;

    std::array<double, 2> delayEffectValue;

    std::array<LinearSmoothedValue<float>, 2> delayFeedbackVolume;
    dsp::FirstOrderTPTFilter<float> lowpass;
    dsp::DryWetMixer<float> mixer;
    std::array<float, 2> lastDelayEffectOutput;
    static constexpr auto effectDelaySamples = 192000;
    dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Linear> linear { effectDelaySamples };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EZDLayAudioProcessor)
};
