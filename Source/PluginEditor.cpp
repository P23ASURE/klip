/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KlipAudioProcessorEditor::KlipAudioProcessorEditor(KlipAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), processor(p)
{
    // ComboBox per selezionare il tipo di Clipping
    clipTypeComboBox.addItem("Soft Clip", 1);
    clipTypeComboBox.addItem("Hard Clip", 2);
    clipTypeComboBox.addItem("Linear Clip", 3);
    clipTypeComboBox.addItem("Exponential Clip", 4);
    clipTypeComboBox.addItem("Asymmetric Clip", 5);
    addAndMakeVisible(&clipTypeComboBox);

    // Rotary Slider per Threshold
    thresholdSlider.setSliderStyle(juce::Slider::Rotary);
    thresholdSlider.setRange(0.0, 1.0, 0.01);
    thresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&thresholdSlider);

    // ComboBox per Mid/Side Processing
    msProcessingComboBox.addItem("Mid", 1);
    msProcessingComboBox.addItem("Side", 2);
    msProcessingComboBox.addItem("Mid+Side", 3);
    addAndMakeVisible(&msProcessingComboBox);

    // Inizializzazione degli Attachment
    clipTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.getParameters(), "clipType", clipTypeComboBox);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getParameters(), "threshold", thresholdSlider);
    msProcessingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.getParameters(), "msProcessing", msProcessingComboBox);

    // Inizializzazione decibelLabel
    decibelLabel.setFont(juce::Font(15.0f));
    decibelLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    decibelLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&decibelLabel);

    // Configurazione FlexBox
    mainFlexBox.flexDirection = juce::FlexBox::Direction::column;
    mainFlexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    mainFlexBox.alignItems = juce::FlexBox::AlignItems::stretch;
    mainFlexBox.items.add(juce::FlexItem(clipTypeComboBox).withFlex(1));
    mainFlexBox.items.add(juce::FlexItem(thresholdSlider).withFlex(2));
    mainFlexBox.items.add(juce::FlexItem(decibelLabel).withFlex(1));
    mainFlexBox.items.add(juce::FlexItem(msProcessingComboBox).withFlex(1));

    setSize(800, 400);
}

KlipAudioProcessorEditor::~KlipAudioProcessorEditor()
{
}

//==============================================================================
void KlipAudioProcessorEditor::paint(juce::Graphics& g)
{

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);

    g.drawFittedText("Klip:: Audio Clipping Plugin", getLocalBounds(), juce::Justification::centredTop, 1);
}


void KlipAudioProcessorEditor::resized()
{
    // Utilizza il FlexBox per posizionare i componenti
    mainFlexBox.performLayout(getLocalBounds());
}
