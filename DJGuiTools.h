/*
  ==============================================================================

    DJGuiTools.h
    Created: 10 Sep 2022 6:17:42pm
    Author:  David Jones

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"
/*
void setSliderParametersGain (juce::Slider& slider, juce::Label& label, const juce::String& labelText, bool hasTextBox = false)
    {
        slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        if (hasTextBox)
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
        else
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 25);
        addAndMakeVisible(slider);
        
        label.setText (labelText, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    }
    void sliderResizedGain (juce::Slider& slider, juce::Label& label, int x, int y)
    {
        const auto sliderToLabelRatio = 15;
        const auto labelHeight = 17;
        const auto sliderWidth = 175;
        const auto sliderHeight = 15;
        slider.setBounds (x, y + sliderToLabelRatio, sliderWidth, sliderHeight);
        label.setBounds (slider.getX(), y, sliderHeight + 80, labelHeight);
    }
*/

