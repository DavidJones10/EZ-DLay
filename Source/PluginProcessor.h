/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define MAX_DELAY_TIME 2000
#include <math.h>
//==============================================================================
/**
*/
using namespace juce;

//===================================================================================
class EZDLayAudioProcessor  :   public AudioProcessor
                                
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
private:
    float delayTimeSmoothed;
    float feedbackRight;
    float feedbackLeft;
    float *delayBufferRight;
    float *delayBufferLeft;
    int bufferWriteHead;
    int bufferLength;
    float delayTimeSamples;
    float delayReadHead;
    
    IIRFilter filterL;
    IIRFilter filterR;
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
    float lerp (float sample1, float sample2, float inPhase);
    AudioProcessorValueTreeState apvts;
    AudioProcessorValueTreeState::ParameterLayout createParams();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EZDLayAudioProcessor)
};
