/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class KlipAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    KlipAudioProcessorEditor (KlipAudioProcessor&);
    ~KlipAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    

private:
    
    juce::FlexBox mainFlexBox;

    juce::Slider thresholdSlider;
    juce::ComboBox msProcessingComboBox;
    juce::ComboBox clipTypeComboBox;

    juce::Label decibelLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> clipTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> msProcessingAttachment;
    
    KlipAudioProcessor& audioProcessor;
    KlipAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KlipAudioProcessorEditor)
};
