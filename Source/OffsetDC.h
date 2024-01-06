/*
  ==============================================================================

    OffsetDC.h
    Created: 4 Jan 2024 9:39:00am
    Author:  Marco

  ==============================================================================
*/

#pragma once
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

class OffsetDCRemover {
public:
    OffsetDCRemover() : x_prev(0.0f), y_prev(0.0f), alpha(0.0f) {}

    void setSampleRate(float sampleRate) {
        const float cutoffFrequency = 40.0f; // Frequenza di taglio per il filtro
        float RC = 1.0f / (cutoffFrequency * 2.0f * M_PI);
        alpha = RC / (RC + 1.0f / sampleRate);
    }

    float processSample(float x) {
        float y = alpha * (y_prev + x - x_prev);
        x_prev = x;
        y_prev = y;
        return y;
    }

private:
    float x_prev, y_prev;
    float alpha;
};