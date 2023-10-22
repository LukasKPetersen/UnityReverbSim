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
    // add localization parameters
    addParameter(gain = new juce::AudioParameterFloat(juce::ParameterID("gain", 1),
                                                      "Gain",
                                                      0.0f,
                                                      1.0f,
                                                      1.0f));
    addParameter(pan = new juce::AudioParameterFloat(juce::ParameterID("pan", 1),
                                                      "Pan",
                                                      -1.0f,
                                                      1.0f,
                                                      0.0f));
    
    // add diffusion parameters
    addParameter(diffusionLevel = new juce::AudioParameterFloat(juce::ParameterID("diffusionLevel", 1),
                                                                "Diffusion Level",
                                                                0.0f,
                                                                1.0f,
                                                                0.5f));
    
    // add delay parameters
    addParameter(delayLevel = new juce::AudioParameterFloat(juce::ParameterID("delayLevel", 1),
                                                                 "Delay Level",
                                                                 0.0f,
                                                                 1.0f,
                                                                 0.5f));
    addParameter(delayTimeLeft = new juce::AudioParameterFloat(juce::ParameterID("delayTimeLeft", 1),
                                                           "Delay Time Left",
                                                           0.01f,
                                                           1.99f,
                                                           0.5f));
    addParameter(delayTimeRight = new juce::AudioParameterFloat(juce::ParameterID("delayTimeRight", 1),
                                                           "Delay Time Right",
                                                           0.01f,
                                                           1.99f,
                                                           0.5f));
    
    // add filter parameters
    addParameter(filterLevel = new juce::AudioParameterFloat(juce::ParameterID("filterLevel", 1),
                                                             "Filter Level",
                                                             0.0f,
                                                             1.0f,
                                                             0.5f));
    
    // add fx parameters
    addParameter(wetLevel = new juce::AudioParameterFloat(juce::ParameterID("wetLevel", 1),
                                                          "Wet level",
                                                          0.0f,
                                                          1.0f,
                                                          0.8f));
    addParameter(dryLevel = new juce::AudioParameterFloat(juce::ParameterID("dryLevel", 1),
                                                          "Dry level",
                                                          0.0f,
                                                          1.0f,
                                                          1.0f));
    addParameter(feedback = new juce::AudioParameterFloat(juce::ParameterID("feedback", 1),
                                                          "Feedback",
                                                          0.0f,
                                                          0.99f,
                                                          0.0f));
    addParameter(lowPassFreq = new juce::AudioParameterFloat(juce::ParameterID("lowPassFreq", 1),
                                                             "Lowpass Frequency",
                                                             2e2,
                                                             4e3,
                                                             1e3));
    
    // we set panSmoother = 0.5 and not 0.0 since JUCE variables are interpreted as values between 0 and 1 in Unity
    panSmoother = 0.5f;
    gainSmoother = 1.0f;
    
    // TODO: make it so that this function is only called on player movement
    applyAudioPositioning = [&] (float panInfo, float frontBackInfo, float distance)
    {
        jassert(distance != 0.0f);
        
        // instead of taking the direct value from Unity we apply smoothening to avoid audio artifacts
        gainSmoother -= 0.02 * (gainSmoother - 1 / distance); // amplitude is inversely proportional to distance
        panSmoother -= 0.01 * (panSmoother - panInfo);

        // set the value parameter based on the Unity input
        getParameters()[0]->setValue(gainSmoother);
        getParameters()[1]->setValue(panSmoother);
        
        diffusion.adjustDiffusionSize(distance / 343.0f * 4.0f); // the last factor is just applyied for audiable effect
        
        filter.setHeadShadowFilter(panInfo, frontBackInfo);
        filter.setDistanceFilter(distance);
    };
    
    clearEchoes = [&] ()
    {
        delay.clearEchoes();
    };
    
    applyDelay = [&] (float delayInSeconds, float soundReduction)
    {
        // for now, we apply the echoes as if we were working in mono
        delay.addEcho(delayInSeconds, soundReduction, 0);
        delay.addEcho(delayInSeconds, soundReduction, 1);
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
    delay.prepare({ sampleRate, (juce::uint32) samplesPerBlock, 2 });
    diffusion.prepare(sampleRate);
    filter.prepare({ sampleRate, (juce::uint32) samplesPerBlock, 2 });
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
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // set diffusion parameters
    diffusion.setWetLevel(wetLevel->get());
    diffusion.setDryLevel(dryLevel->get());
    
    // set delay parameters
    delay.setFeedback(feedback->get());
    delay.setWetLevel(wetLevel->get());
    delay.setDryLevel(dryLevel->get());
    
    // set filter parameters
    filter.setWetLevel(wetLevel->get());
    filter.setDryLevel(dryLevel->get());
    
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        // set delay time
        delay.setDelayTime(ch, ch == 0 ? delayTimeLeft->get() : delayTimeRight->get());
        delay.setLowpassFreq(ch, lowPassFreq->get());
        
        auto* channelReadData = buffer.getReadPointer (ch);
        
        // we apply diffusion to the channel input signal
//        std::vector<float> processedSignal = diffusion.processChannelBuffer(ch, channelReadData, buffer.getNumSamples());
        std::vector<float> diffusedSignal = diffusion.processChannelBuffer(ch, channelReadData, buffer.getNumSamples()); // just for debugging purposes
        jassert (diffusedSignal.size() == buffer.getNumSamples());
        
        // we apply delay processing to the channel input signal
//        processedSignal = delay.processChannelBuffer(ch, channelReadData, buffer.getNumSamples());
        std::vector<float> delayedSignal = delay.processChannelBuffer(ch, channelReadData, buffer.getNumSamples()); // just for debugging purposes
        jassert (delayedSignal.size() == buffer.getNumSamples());
        
        // we apply filtering to the channel input signal
        std::vector<float> filteredSignal = filter.processChannelBuffer(ch, channelReadData, buffer.getNumSamples());
        jassert (filteredSignal.size() == buffer.getNumSamples());
        
        // calculate the pan value (channel-wise)
        float panValue = ch == 0 ?  juce::jmap(pan->get(), -1.0f, 1.0f, -1.0f, 0.0f) * (-1) :
                                    juce::jmap(pan->get(), -1.0f, 1.0f, 0.0f, 1.0f);
        
        auto* channelWriteData = buffer.getWritePointer (ch);
        
        // set each sample of the output buffer
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelWriteData[sample] = std::tanh (diffusionLevel->get() * diffusedSignal[sample] + delayLevel->get() * delayedSignal[sample] + filterLevel->get() * filteredSignal[sample]) * panValue * gain->get();
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
    return new juce::GenericAudioProcessorEditor (*this);
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
