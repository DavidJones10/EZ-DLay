/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EZDLayAudioProcessor::EZDLayAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
      AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
apvts(*this,
      nullptr,
      "Parameters",
{
std::make_unique<AudioParameterFloat>(ParameterID("FEEDBACK", 1), "Feedback", NormalisableRange<float> { 0.0f, .98f, .001f }, 0.5f),
std::make_unique<AudioParameterFloat>(ParameterID("DELAYTIME",1), "Delay Time", NormalisableRange<float> { 0.0f, MAX_DELAY_TIME, 0.1f }, 200.0f) ,
std::make_unique<AudioParameterFloat>(ParameterID("MIX",1), "Mix", NormalisableRange<float> { 0.0f, 1.0f, .001f }, 0.5f),
std::make_unique<AudioParameterFloat>(ParameterID("CUTOFF",1), "Filter Cutoff Freq", NormalisableRange<float> { 20.0f, 20000.0f, .1f }, 20000.0f)
}
               )
#endif
{
    delayBufferRight = nullptr;
    delayBufferLeft = nullptr;
    
    bufferWriteHead = 0;
    bufferLength = 0;
    bufferLength = 0;
    
    delayTimeSamples = 0;
    delayReadHead = 0;
    delayTimeSmoothed = 0;
    
    feedbackLeft = 0;
    feedbackRight = 0;
}

EZDLayAudioProcessor::~EZDLayAudioProcessor()
{
    if (delayBufferRight != nullptr)
    {
        delete [] delayBufferRight;
        delayBufferRight = nullptr;
    }
    if (delayBufferLeft != nullptr)
    {
        delete [] delayBufferLeft;
        delayBufferLeft = nullptr;
    }
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
    filterL.reset();
    filterR.reset();
    
    filterL.setCoefficients(IIRCoefficients::makeLowPass(sampleRate, 20000));
    filterR.setCoefficients(IIRCoefficients::makeLowPass(sampleRate, 20000));
    
    bufferLength = sampleRate * (MAX_DELAY_TIME / 1000);
    delayTimeSamples = sampleRate * (*apvts.getRawParameterValue("DELAYTIME")/1000);
    if (delayBufferLeft == nullptr)
        {
            delete [] delayBufferLeft;
            delayBufferLeft = new float[bufferLength];
        }
    zeromem(delayBufferLeft, bufferLength * sizeof(float));
    
    if (delayBufferRight == nullptr)
        {
            delete [] delayBufferRight;
            delayBufferRight = new float[bufferLength];
        }

    // Clear the buffers after instantiating!
    zeromem(delayBufferRight, bufferLength * sizeof(float));

    bufferWriteHead = 0;
    delayTimeSmoothed = (*apvts.getRawParameterValue("DELAYTIME")/1000);
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
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto& dTime = *apvts.getRawParameterValue("DELAYTIME");
    auto& fBack = *apvts.getRawParameterValue("FEEDBACK");
    auto& mix = *apvts.getRawParameterValue("MIX");
    auto& cutoff = *apvts.getRawParameterValue("CUTOFF");
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    float* leftBuffer = buffer.getWritePointer(0);
    float* rightBuffer = buffer.getWritePointer(1);
    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
    {
        filterL.setCoefficients(IIRCoefficients::makeLowPass(getSampleRate(), cutoff));
        filterR.setCoefficients(IIRCoefficients::makeLowPass(getSampleRate(), cutoff));
        
        delayTimeSmoothed = delayTimeSmoothed - .0001 * (delayTimeSmoothed - (dTime/1000));
        delayTimeSamples = getSampleRate() * delayTimeSmoothed;
        
        delayBufferRight[bufferWriteHead] = rightBuffer[sample] + feedbackRight;
        delayBufferLeft[bufferWriteHead] = leftBuffer[sample] + feedbackLeft;

        
        delayReadHead = bufferWriteHead - delayTimeSamples;
        
        if (delayReadHead < 0)
            delayReadHead += bufferLength;
        
        int readHeadInt = (int) delayReadHead;
        int readHeadInt1 = readHeadInt + 1;
        float readHeadFloat = delayReadHead - readHeadInt;
        
        if (readHeadInt1 >= bufferLength)
            readHeadInt1 -= bufferLength;
        
        float delaySampleLeft = lerp(delayBufferLeft[readHeadInt], delayBufferLeft[readHeadInt1], readHeadFloat);
        float delaySampleRight = lerp(delayBufferRight[readHeadInt], delayBufferRight[readHeadInt1], readHeadFloat);
        float delaySampleLowPassL = filterL.processSingleSampleRaw(delaySampleLeft);
        float delaySampleLowPassR = filterR.processSingleSampleRaw(delaySampleRight);

        feedbackLeft = fBack * delaySampleLowPassL;
        feedbackRight = fBack * delaySampleLowPassR;
        
        bufferWriteHead++;
        
        buffer.setSample(0, sample, buffer.getSample(0, sample) * (1-mix) + delaySampleLowPassL * mix);
        buffer.setSample(1, sample, buffer.getSample(1, sample) * (1-mix) + delaySampleLowPassR * mix);

        if (bufferWriteHead >= bufferLength)
            bufferWriteHead = 0;
        }
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
  //============//
 //MY FUNCTIONS//
//============//

float EZDLayAudioProcessor::lerp(float sample1, float sample2, float inPhase)
{
    return (1 - inPhase) * sample1 + inPhase * sample2;
}

