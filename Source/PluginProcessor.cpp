/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KlipAudioProcessor::KlipAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "parameters", juce::AudioProcessorValueTreeState::ParameterLayout{
    // Definizione dei parametri utilizzando AudioProcessorValueTreeState
    std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f),
    std::make_unique<juce::AudioParameterChoice>("clipType", "Clip Type", juce::StringArray{ "Soft Clip", "Hard Clip", "Linear Clip", "Exponential Clip", "Asymmetric Clip" }, 0),
    std::make_unique<juce::AudioParameterChoice>("msProcessing", "MS Processing", juce::StringArray{ "Mid", "Side", "Mid+Side" }, 2)
        })
#endif
{

}

KlipAudioProcessor::~KlipAudioProcessor()
{
}
//==============================================================================

float KlipAudioProcessor::convertToDecibel(float sliderValue)
{
    return sliderValue * 24.0f - 24.0f; // Mappa 
}

// =======================================Mid+Side PhaseControl=========================================================================

void KlipAudioProcessor::initializeAllPassFilters()
{
    auto allPassCoefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(44100, 1000.0f);
    leftAllPassFilter.coefficients = allPassCoefficients;
    rightAllPassFilter.coefficients = allPassCoefficients;
}

std::pair<float, float> KlipAudioProcessor::combineMidSideWithPhaseControl(float mid, float side)
{
    // Applica i filtri all-pass ai segnali mid e side
    float left = leftAllPassFilter.processSample(mid + side);
    float right = rightAllPassFilter.processSample(mid - side);

    return std::make_pair(left, right);
}

// ===========================mid/side processing===========================================

void KlipAudioProcessor::processMid(juce::AudioBuffer<float>& buffer, Clipping::ClipType clipType) {
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float mid = 0.5f * (buffer.getSample(0, sample) + buffer.getSample(1, sample));
        float midClipped = clipping.processSample(mid, clipType);
        buffer.setSample(0, sample, midClipped);
        buffer.setSample(1, sample, midClipped);
    }
}

void KlipAudioProcessor::processSide(juce::AudioBuffer<float>& buffer, Clipping::ClipType clipType) {
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float side = 0.5f * (buffer.getSample(0, sample) - buffer.getSample(1, sample));
        float sideClipped = clipping.processSample(side, clipType);

        buffer.setSample(0, sample, sideClipped);
        buffer.setSample(1, sample, -sideClipped);
    }
}

void KlipAudioProcessor::processMidSide(juce::AudioBuffer<float>& buffer, Clipping::ClipType clipType) {
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float mid = 0.5f * (buffer.getSample(0, sample) + buffer.getSample(1, sample));
        float side = 0.5f * (buffer.getSample(0, sample) - buffer.getSample(1, sample));

        // Process the Mid and Side components
        float midClipped = clipping.processSample(mid, clipType);
        float sideClipped = clipping.processSample(side, clipType);

        // Combine processed Mid and Side back into Left and Right channels
        buffer.setSample(0, sample, midClipped + sideClipped); // Left
        buffer.setSample(1, sample, midClipped - sideClipped); // Right
    }
}


// ==============================================================================
const juce::String KlipAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KlipAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool KlipAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool KlipAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double KlipAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KlipAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int KlipAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KlipAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String KlipAudioProcessor::getProgramName(int index)
{
    return {};
}

void KlipAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void KlipAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    clipping.setSampleRate(static_cast<float>(sampleRate));
    const double timeDuration = 0.05; // 50 milliseconds
    int bufferSize = static_cast<int>(sampleRate * timeDuration);

    clipping.setupLowFrequencyAnalysis(sampleRate, bufferSize);
    initializeAllPassFilters();
}

void KlipAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KlipAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void KlipAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();

    // Ottieni i valori dei parametri
    auto currentThreshold = parameters.getRawParameterValue("threshold")->load();
    auto msChoiceValue = parameters.getRawParameterValue("msProcessing")->load();
    auto clipTypeChoiceValue = parameters.getRawParameterValue("clipType")->load();

    // Converti i valori float in indici interi (errore in atomic)
    int msChoice = static_cast<int>(msChoiceValue);
    int clipTypeChoice = static_cast<int>(clipTypeChoiceValue);

    // Ottimizzazione: Valori di soglia memorizzati nella cache
    static float cachedThresholdInDecibels = -1.0f;
    if (currentThreshold != cachedThresholdInDecibels) {
        cachedThresholdInDecibels = currentThreshold;
        thresholdInDecibels = convertToDecibel(cachedThresholdInDecibels);
        clipping.setThreshold(thresholdInDecibels);
    }

    Clipping::ClipType clipType;
    switch (clipTypeChoice) {
    case 0: clipType = Clipping::SoftClip; break;
    case 1: clipType = Clipping::HardClip; break;
    case 2: clipType = Clipping::LinearClip; break;
    case 3: clipType = Clipping::ExponentialClip; break;
    case 4: clipType = Clipping::AsymmetricClip; break;
    default: clipType = Clipping::SoftClip;
    }

    if (totalNumInputChannels >= 2) {
        switch (msChoice) {
        case 0:
            processMid(buffer, clipType);
            break;
        case 1:
            processSide(buffer, clipType);
            break;
        case 2:
            processMidSide(buffer, clipType);
            break;
        }
    }

    for (int channel = totalNumInputChannels; channel < getTotalNumOutputChannels(); ++channel) {
        buffer.clear(channel, 0, buffer.getNumSamples());
    }
}


//==============================================================================
bool KlipAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KlipAudioProcessor::createEditor()
{
    return new KlipAudioProcessorEditor(*this);
}

//==============================================================================
void KlipAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Salva lo stato corrente in un flusso di dati
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void KlipAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Ripristina lo stato da un flusso di dati
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}





//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KlipAudioProcessor();
}
