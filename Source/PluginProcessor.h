/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Clipping.h"
// #include "OffsetDC.h"
//==============================================================================
/**
*/
class KlipAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    KlipAudioProcessor();
    ~KlipAudioProcessor() override;

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
    
    float convertToDecibel(float sliderValue);   
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    void processMid(juce::AudioBuffer<float>& buffer, Clipping::ClipType clipType);
    void processSide(juce::AudioBuffer<float>& buffer, Clipping::ClipType clipType);
    void processMidSide(juce::AudioBuffer<float>& buffer, Clipping::ClipType clipType);
 
    std::pair<float, float> combineMidSideWithPhaseControl(float mid, float side);
    juce::dsp::IIR::Filter<float> leftAllPassFilter;
    juce::dsp::IIR::Filter<float> rightAllPassFilter;
    void initializeAllPassFilters();

    float thresholdInDecibels;

    Clipping clipping;

    juce::AudioProcessorValueTreeState parameters;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KlipAudioProcessor)
};
