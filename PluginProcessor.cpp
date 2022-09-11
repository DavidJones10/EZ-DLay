/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EZDLayAudioProcessor::EZDLayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

EZDLayAudioProcessor::~EZDLayAudioProcessor()
{
}

//==============================================================================
const juce::String EZDLayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EZDLayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EZDLayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EZDLayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EZDLayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EZDLayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EZDLayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EZDLayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EZDLayAudioProcessor::getProgramName (int index)
{
    return {};
}

void EZDLayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EZDLayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void EZDLayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EZDLayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EZDLayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        
        auto* channelData = buffer.getWritePointer (channel);
        mixer.pushDrySamples(buffer);
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto input = channelData[sample] - lastDelayEffectOutput[channel];
            auto delay = smoothFilter.processSample(channel, delayEffectValue[channel]);
            const auto output = [&]
            {
                linear.pushSample (channel, input);
                linear.setDelay (delay);
                linear.popSample (channel);
                jassertfalse;
                return 0.0f;
            }();
            const auto processed = lowpass.processSample (int (channel), output);
            channelData[sample] = processed;
            lastDelayEffectOutput[channel] = processed * delayFeedbackVolume[channel].getNextValue();
        }
        channelData[channel]  = lastDelayEffectOutput[channel];
    }
    mixer.mixWetSamples(buffer);
}

//==============================================================================
bool EZDLayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EZDLayAudioProcessor::createEditor()
{
    return new EZDLayAudioProcessorEditor (*this);
}

//==============================================================================
void EZDLayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EZDLayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EZDLayAudioProcessor();
}
  //=============//
 //MY FUNCTIONS //
//=============//
