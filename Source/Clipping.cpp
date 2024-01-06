/*
  ==============================================================================

    Clipping.cpp
    Created: 3 Jan 2024 10:20:39am
    Author:  Marco

  ==============================================================================
*/
#include "Clipping.h"
#include <cmath>


void Clipping::setSampleRate(float newSampleRate) {
    this->sampleRate = sampleRate;
    dcRemover.setSampleRate(newSampleRate);
}

void Clipping::setThreshold(float newThreshold) {
    threshold = newThreshold;
}

void Clipping::startTransitionTo(ClipType newType, float speed) {
    if (newType != currentClipType) {
        newClipType = newType;
        transitionSpeed = speed;
        transitionState = 0.0f;
    }
}

void Clipping::updateTransition() {
    if (transitionState < 1.0f) {
        transitionState += transitionSpeed;
        if (transitionState > 1.0f) transitionState = 1.0f;
    }
}

float Clipping::mixClippingFunctions(float input) {
    float currentClip = processClip(input, currentClipType);
    float newClip = processClip(input, newClipType);
    return currentClip * (1.0f - transitionState) + newClip * transitionState;
}

float Clipping::calculateLowFrequencyEnergy(float sample) {
    // Update ring buffer
    ringBuffer[ringBufferIndex] = sample;
    ringBufferIndex = (ringBufferIndex + 1) % ringBuffer.size();

    // Apply low-pass filter to the buffer and calculate energy
    float squaredSum = 0.0f;
    for (auto& s : ringBuffer) {
        float filteredSample = lowPassFilter.processSample(s);
        squaredSum += filteredSample * filteredSample;
    }

    return std::sqrt(squaredSum / ringBuffer.size());
}

float Clipping::calculateDynamicGain(float lowFreqEnergy, float overallEnergy) {
    // Constants for dynamic behavior - these may still need fine-tuning
    const float lowFreqEnergyThreshold = 0.1f; // Threshold for excessive low-frequency energy
    const float maxGainReduction = 0.5f; // Maximum gain reduction when excessive low-frequency is detected
    const float gainRecoveryRate = 0.05f; // Rate at which the gain recovers back to normal

    // Dynamic gain calculation based on low-frequency energy
    float dynamicGain;
    if (lowFreqEnergy > lowFreqEnergyThreshold) {
        // Calculate gain reduction factor based on how much the low frequency energy exceeds the threshold
        float excessRatio = (lowFreqEnergy - lowFreqEnergyThreshold) / lowFreqEnergyThreshold;
        dynamicGain = 1.0f - std::min(excessRatio, 1.0f) * maxGainReduction;
    }
    else {
        dynamicGain = 1.0f; // No gain reduction if low-frequency energy is below the threshold
    }

    // Implementing a smoothing mechanism for gradual gain changes
    static float smoothedGain = 1.0f;
    if (dynamicGain < smoothedGain) {
        smoothedGain = dynamicGain; // Quick reduction if needed
    }
    else {
        smoothedGain += gainRecoveryRate * (dynamicGain - smoothedGain); // Gradual increase to normal gain
    }

    return juce::jlimit(0.0f, 1.0f, smoothedGain); // Ensuring gain stays within bounds
}

void Clipping::setupLowFrequencyAnalysis(int sampleRate, int bufferSize) {
    auto lowPassCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 40.0f);
    lowPassFilter.coefficients = lowPassCoeff;
    ringBuffer.resize(bufferSize, 0.0f);
    ringBufferIndex = 0;
}


// =====================================================================================================

float Clipping::processSample(float input, ClipType clipType) {
    // Applicazione del filtro di rimozione dell'offset DC
    input = dcRemover.processSample(input);

    // Se l'input è estremamente vicino a zero, ritorna zero
    if (std::abs(input) < 1e-8) {
        return 0.0f;
    }

    // Se il tipo di clip è cambiato, inizia la transizione
    if (clipType != currentClipType) {
        startTransitionTo(clipType, 0.05f); // !! regolare la velocità di transizione come necessario
        currentClipType = clipType;
    }

    // Aggiorna lo stato di transizione
    updateTransition();

    // Combina le funzioni di clipping in base allo stato di transizione
    return mixClippingFunctions(input);
}


float Clipping::processClip(float input, ClipType clipType) {
    switch (clipType) {
    case SoftClip: return softClip(input);
    case HardClip: return hardClip(input);
    case LinearClip: return linearClip(input);
    case ExponentialClip: return exponentialClip(input);
    case AsymmetricClip: return asymmetricClip(input);
    
    default: return input;
    }
}
// =============================================================================================================================================

float Clipping::softClip(float input) {
    if (input > threshold) {
        return (1.5f * input - 0.5f * input * input * input / threshold) / threshold;
    }
    else if (input < -threshold) {
        return (-1.5f * input - 0.5f * input * input * input / threshold) / threshold;
    }
    else {
        return input;
    }
}

float Clipping::hardClip(float input) {
    if (input > threshold) {
        return threshold;
    }
    else if (input < -threshold) {
        return -threshold;
    }
    else {
        return input;
    }
}

float Clipping::linearClip(float input) {
    if (input > threshold) {
        return threshold + 0.5f * (input - threshold); // Transizione graduale oltre la soglia
    }
    else if (input < -threshold) {
        return -threshold + 0.5f * (input + threshold); // Transizione graduale sotto la soglia
    }
    else {
        return input;
    }
}

float Clipping::exponentialClip(float input) {
    if (input > threshold) {
        return threshold + (1 - exp(-((input - threshold) / threshold) * 3)) * (threshold / 4);
    }
    else if (input < -threshold) {
        return -threshold - (1 - exp(((input + threshold) / threshold) * 3)) * (threshold / 4);
    }
    else {
        return input;
    }
}

float Clipping::asymmetricClip(float input) {
    if (input > threshold) {
        // Curva modificata per valori positivi
        return threshold + log(1 + (input - threshold)) / log(2);
    }
    else if (input < -threshold) {
        // Curva diversa e più morbida per valori negativi
        return -threshold - log(1 + (-input - threshold)) / log(3);
    }
    else {
        return input;
    }
}
