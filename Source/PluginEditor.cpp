/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
EZDLayAudioProcessorEditor::EZDLayAudioProcessorEditor (EZDLayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&otherLookAndFeel);
    setSize(350, 360);
    Timer::startTimerHz(20);

    feedbackSlider.setLookAndFeel(&otherLookAndFeel);
    delayTimeSlider.setLookAndFeel(&otherLookAndFeel);
    lowpassFreqSlider.setLookAndFeel(&otherLookAndFeel);
    mixSlider.setLookAndFeel(&otherLookAndFeel);
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    feedbackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FEEDBACK", feedbackSlider);
    delayTimeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DELAYTIME", delayTimeSlider);
    lowpassAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CUTOFF", lowpassFreqSlider);
    mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MIX", mixSlider);

    
    setSliderParametersDial(feedbackSlider);
    setSliderParametersDial(delayTimeSlider);
    setSliderParametersDial(lowpassFreqSlider);
    setSliderParametersDial(mixSlider);
    
    feedbackSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    delayTimeSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    lowpassFreqSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    mixSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    


}

EZDLayAudioProcessorEditor::~EZDLayAudioProcessorEditor()
{
    setLookAndFeel((nullptr));
}

//==============================================================================
void EZDLayAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto sliderPosFeedback = feedbackSlider.getValue() * (pi / (pi+.07))/ feedbackSlider.getMaximum();
    auto sliderPosDelayTime = delayTimeSlider.getValue() / delayTimeSlider.getMaximum();
    auto sliderPosLowpass = lowpassFreqSlider.getValue() / lowpassFreqSlider.getMaximum();
    auto sliderPosMix = mixSlider.getValue() / mixSlider.getMaximum();
    auto titleFont = Font("Euphemia UCAS", 60.0f, Font::plain);
    
    g.fillAll (Colours::black);
    g.setFont(titleFont);
    g.setColour(Colours::skyblue);
    g.setOpacity(1);
    g.drawFittedText(String("EZ DLay"), 150, 0, 200, 60, Justification::centredTop, 1);
        
    drawGroupRectangle(delayTimeSlider, feedbackSlider, String("Stuff"), g);
    drawGroupRectangle(mixSlider, lowpassFreqSlider, String("Stuff"), g);
       
    drawRotarySlider(g, row1X, column1Y, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosMix, 4 * pi / 3, 8 * pi /3, mixSlider, String("Mix"));
    drawRotarySlider(g, mixSlider.getRight() + horizontalDistance, column1Y, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosDelayTime, 4 * pi / 3, 8 * pi /3, delayTimeSlider, String("Delay Time"));
    drawRotarySlider(g, delayTimeSlider.getX(), delayTimeSlider.getBottom() + distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosFeedback , 4 * pi / 3, 8 * pi /3, feedbackSlider, String("Feedback"));
    drawRotarySlider(g, mixSlider.getX(), feedbackSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosLowpass, 4 * pi / 3, 8 * pi /3, lowpassFreqSlider, String("Cutoff Freq"));
    drawParamText(g);

}

void EZDLayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    mixSlider.setBounds(row1X, column1Y, sliderWidthAndHeight, sliderWidthAndHeight);
    delayTimeSlider.setBounds(mixSlider.getRight() + horizontalDistance, column1Y, sliderWidthAndHeight, sliderWidthAndHeight);
    feedbackSlider.setBounds(delayTimeSlider.getX(), delayTimeSlider.getBottom() + distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight);
    lowpassFreqSlider.setBounds(mixSlider.getX(), feedbackSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight);
}

void EZDLayAudioProcessorEditor::drawParamText(Graphics &g)
{
    auto text = String("");
    if (feedbackSlider.isMouseOverOrDragging())
    {
        auto& fBack = *audioProcessor.apvts.getRawParameterValue("FEEDBACK");
        text = String("Feedback:     " + std::to_string(fBack));
    }
    if (mixSlider.isMouseOverOrDragging())
    {
        auto& mix = *audioProcessor.apvts.getRawParameterValue("MIX");
        text = String("Mix:           " + std::to_string(mix));
    }
    if (delayTimeSlider.isMouseOverOrDragging())
    {
        auto& dTime = *audioProcessor.apvts.getRawParameterValue("DELAYTIME");
        text = String("Delay Time: " + std::to_string(dTime) + "ms");
    }
    if (lowpassFreqSlider.isMouseOverOrDragging())
    {
        auto& lpFreq = *audioProcessor.apvts.getRawParameterValue("CUTOFF");
        text = String("Lowpass Frequency: " + std::to_string(lpFreq) + "Hz");
    }
    auto textRect = Rectangle<float>(2, 10, 150, 50);
    g.setColour(Colours::white);
    g.drawFittedText(text, 5, 10, 140, 50, Justification::centredTop, 2);
    g.drawRoundedRectangle(textRect, 10, 2);
    g.setColour(Colours::grey);
    g.setOpacity(.5);
    g.drawRoundedRectangle(textRect, 10, 2);
}
void EZDLayAudioProcessorEditor::timerCallback()
{
    repaint();
}
