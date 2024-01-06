/*
  ==============================================================================

    Clipping.h
    Created: 3 Jan 2024 10:20:39am
    Author:  Marco

  ==============================================================================
*/
#pragma once
#include <algorithm>
#include <JuceHeader.h>
#include "OffsetDC.h"

class Clipping {
public:
    Clipping() = default; // Costruttore di default
     ~Clipping() = default;

     void setSampleRate(float sampleRate);

    enum ClipType {
        SoftClip,
        HardClip,
        LinearClip,
        ExponentialClip,
        AsymmetricClip,
        SaturationClip
        // Aggiungi altri tipi di clipping qui
    };

    void setThreshold(float newThreshold);
    float processSample(float input, ClipType clipType);
    float processClip(float input, ClipType clipType);

    // smoothing transition
    void startTransitionTo(ClipType newType, float speed);
    void updateTransition();
    float mixClippingFunctions(float input);
    
    void setupLowFrequencyAnalysis(int sampleRate, int bufferSize);
    float calculateLowFrequencyEnergy(float sample);
    float calculateDynamicGain(float lowFreqEnergy, float overallEnergy);
private:
    OffsetDCRemover dcRemover;

    float threshold = 0.0f; 

    float transitionState = 0.0f;
    float transitionSpeed = 0.05f; // Adjust this value as needed
    ClipType currentClipType = SoftClip;
    ClipType newClipType = SoftClip;

    
    juce::dsp::IIR::Filter<float> lowPassFilter;
    std::vector<float> ringBuffer;
    int ringBufferIndex = 0;
    
  
    float softClip(float input);
    float hardClip(float input);
    float linearClip(float input);
    float exponentialClip(float input);
    float asymmetricClip(float input);
    
    float sampleRate;
};

