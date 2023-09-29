/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================
SpatiotemporalReverbAudioProcessor::SpatiotemporalReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : MyAudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(gain = new juce::AudioParameterFloat(juce::ParameterID("gain", 1),
                                                      "Gain",
                                                      0.0f,
                                                      1.0f,
                                                      0.0f));
    addParameter(pan = new juce::AudioParameterFloat(juce::ParameterID("pan", 1),
                                                      "Pan",
                                                      -1.0f,
                                                      1.0f,
                                                      0.0f));
    
    // we set panSmoother = 0.5 since JUCE variables are interpreted as values between 0 and 1 in Unity
    panSmoother = 0.5f;
    gainSmoother = 0.5f;
    
    applyAudioPositioning = [&] (float panInfo, float amplitude)
    {
        // instead of taking the direct value from Unity we apply smoothening to avoid audio artifacts
        gainSmoother -= 0.02 * (gainSmoother - amplitude);
        panSmoother -= 0.01 * (panSmoother - panInfo);

        // set the value parameter based on the Unity input
        getParameters()[0]->setValue(gainSmoother);
        getParameters()[1]->setValue(panSmoother);
    };
}

SpatiotemporalReverbAudioProcessor::~SpatiotemporalReverbAudioProcessor()
{
}

//==============================================================================
const juce::String SpatiotemporalReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpatiotemporalReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpatiotemporalReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpatiotemporalReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpatiotemporalReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpatiotemporalReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpatiotemporalReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpatiotemporalReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpatiotemporalReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpatiotemporalReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpatiotemporalReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SpatiotemporalReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpatiotemporalReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

void SpatiotemporalReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // calculate the pan value
        float panValue = channel == 0 ? juce::jmap(pan->get(),
                                                   -1.0f,
                                                   1.0f,
                                                   -1.0f,
                                                   0.0f) * (-1) :
                                        juce::jmap(pan->get(),
                                                   -1.0f,
                                                   1.0f,
                                                   0.0f,
                                                   1.0f);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= panValue * gain->get(); // apply both panning and gain to each sample
        }
    }
}

//==============================================================================
bool SpatiotemporalReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpatiotemporalReverbAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SpatiotemporalReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpatiotemporalReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpatiotemporalReverbAudioProcessor();
}

MyAudioProcessor* JUCE_CALLTYPE createPluginFilterNEW()
{
    return new SpatiotemporalReverbAudioProcessor();
}
