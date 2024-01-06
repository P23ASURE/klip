# Klip - Audio Clipping Plugin

## Overview
Klip Audio Processor is an audio plugin developed using the JUCE framework, designed to provide users with control over the audio clipping process. It offers various clipping functions and allows users to adjust the threshold and process the signal in mid, side, and mid+side modes.

## Features
- **Clipping Function Selection**: A ComboBox allows users to choose from different clipping functions, including Soft Clip, Hard Clip, Linear Clip, Exponential Clip, and Asymmetric Clip.
- **Threshold Adjustment**: A Rotary Slider enables the adjustment of the signal's threshold, directly influencing the intensity of the clipping.
- **Processing Mode**: Users can select the signal processing mode (mid, side, mid+side) through another ComboBox.

## Code Structure
The plugin consists of the following main files:
- `plugineditor.cpp/h`: Manages the user interface of the plugin.
- `pluginprocessor.cpp/h`: Handles the audio processing logic of the plugin.
- `clipping.cpp/h`: Contains the implementations of the various clipping functions.
- `dcoffset.h`: Currently acts as a high-pass filter, providing additional signal processing capabilities.

