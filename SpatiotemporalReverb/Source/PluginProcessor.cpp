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
                                                      0.5f));
    addParameter(pan = new juce::AudioParameterFloat(juce::ParameterID("pan", 1),
                                                      "Pan",
                                                      -1.0f,
                                                      1.0f,
                                                      0.0f));
    
    // add delay parameters
    addParameter(feedback = new juce::AudioParameterFloat(juce::ParameterID("feedback", 1),
                                                          "Feedback",
                                                          0.0f,
                                                          0.99f,
                                                          0.0f));
    
    // add fx parameters
    addParameter(wetLevel = new juce::AudioParameterFloat(juce::ParameterID("wetLevel", 1),
                                                          "Wet level",
                                                          0.0f,
                                                          1.0f,
                                                          1.0f));
    addParameter(dryLevel = new juce::AudioParameterFloat(juce::ParameterID("dryLevel", 1),
                                                          "Dry level",
                                                          0.0f,
                                                          1.0f,
                                                          0.0f));
    addParameter(reverbLevel = new juce::AudioParameterFloat(juce::ParameterID("reverbLevel", 1),
                                                             "Reverb Level",
                                                             0.0f,
                                                             1.0f,
                                                             1.0f));
    addParameter(directLevel = new juce::AudioParameterFloat(juce::ParameterID("directLevel", 1),
                                                             "Direct Level",
                                                             0.0f,
                                                             1.0f,
                                                             1.0f));
    
    // add filter parameters
    addParameter(obstructedReflections = new juce::AudioParameterFloat(juce::ParameterID("obstructedReflections", 1),
                                                                       "Obstructed Reflections",
                                                                       0.0f,
                                                                       1.0f,
                                                                       0.0f));
    
    // we set panSmoother = 0.5 and not 0.0 since JUCE variables are interpreted as values between 0 and 1 in Unity
    panSmoother = 0.5f;
    gainSmoother = 0.5f;
    obstructedReflectionsSmoother = 0.0f;
    delayTimeSmoother = 0.0f;
    feedbackSmoother = 0.0f;
    
    // TODO: make it so that this function is only called on player movement, not every frame necessarily
    applyAudioPositioning = [&] (float panInfo, float frontBackInfo, float distance, float transmission, float filterCoefLeft, float filterCoefRight)
    {
        jassert(distance != 0.0f);
        
        // instead of taking the direct value from Unity we apply smoothening to avoid audio artifacts (an S-curve)
        gainSmoother -= 0.02f * (gainSmoother - transmission / distance); // amplitude is inversely proportional to distance
        panSmoother -= 0.01f * (panSmoother - panInfo);
        
        // set the value parameter based on the Unity input
        getParameters()[0]->setValue(gainSmoother);
        getParameters()[1]->setValue(panSmoother);
        
        occlusionFilterLeftCoef -= 0.4f * (occlusionFilterLeftCoef - filterCoefLeft);
        occlusionFilterRightCoef -= 0.4f * (occlusionFilterRightCoef - filterCoefLeft);
        
        // set filters
        setFilterValues(panInfo, frontBackInfo, distance, occlusionFilterLeftCoef, occlusionFilterRightCoef);
    };
    
    setObstructedReflections = [&] (float obstructedReflections)
    {
        jassert (0.0f <= obstructedReflections && obstructedReflections <= 1.0f);
        // apply S-curve
        obstructedReflectionsSmoother -= 0.4f * (obstructedReflectionsSmoother - obstructedReflections);
        getParameters()[7]->setValue (obstructedReflections);
    };
    
    setDiffusionSize = [&] (float diffusionTime)
    {
        // set diffusion amount (no S-curve needed since this is done implicitly in the diffusion class)
        processorChain.template get<diffusionIndex>().adjustDiffusionSize (diffusionTime);
    };
    
    setDelayTime = [&] (float delayTime)
    {
        // apply S-curve
        delayTimeSmoother -= 0.02f * (delayTimeSmoother - delayTime);
        processorChain.template get<delayIndex>().setDelayTimes (delayTimeSmoother);
    };
    
    setFeedback = [&] (float feedback)
    {
        // apply S-curve
        feedbackSmoother -= 0.02f * (feedbackSmoother - feedback);
        processorChain.template get<delayIndex>().setFeedback (feedbackSmoother);
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
    auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    processorChain.prepare (spec);
    filter.prepare(spec);
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
    
    // setup the audio block(s) for processing
    juce::AudioBuffer<float> bufferCopy (2, buffer.getNumSamples());
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        bufferCopy.copyFrom (ch, 0, buffer, ch, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::AudioBlock<float> filterOnlyBlock (bufferCopy);
    
    
    /* SIGNAL 1: Dry Signal -> Diffuser -> Delay -> Filter -> Output */
    // we process the dry signal through diffusion and delay
    juce::dsp::ProcessContextReplacing<float> context (block);
    processorChain.template get<filterIndex>().setWetDryBalance(obstructedReflections->get());
//    processorChain.process (context);
//    processorChain.template get<diffusionIndex>().process (context);
    processorChain.template get<delayIndex>().process (context);
    processorChain.template get<filterIndex>().process (context);
    
    
    /* SIGNAL 2: Dry Signal -> Filter -> Output */
    // we process the direct signal through the filter
    juce::dsp::ProcessContextReplacing<float> filterOnlyContext (filterOnlyBlock);
    filter.setWetDryBalance (1.0f);
    filter.process (filterOnlyContext);
    
    
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        // calculate the channel-wise pan value
        float panValue = ch == 0 ?  juce::jmap(pan->get(), -1.0f, 1.0f, -1.0f, 0.0f) * (-1) :
                                    juce::jmap(pan->get(), -1.0f, 1.0f, 0.0f, 1.0f);
        
        for (int sample = 0; sample < context.getOutputBlock().getNumSamples(); ++sample)
        {
            auto reverbSample = context.getOutputBlock().getSample (ch, sample);
            auto filterSample = filterOnlyContext.getOutputBlock().getSample (ch, sample);
            
            // apply panning and gain control to the processed sample
            auto outputSample = std::tanh (
                                           (reverbLevel->get() * reverbSample + directLevel->get() * filterSample)
                                           * panValue
                                           * gain->get()
                                           );
            context.getOutputBlock().setSample(ch, sample, outputSample);
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

void SpatiotemporalReverbAudioProcessor::setFilterValues(float panInfo, float frontBackInfo, float distance, float occlusionFilterLeftCoef, float occlusionFilterRightCoef) {
    
    // the direct filter
    filter.setHeadShadowFilter(panInfo, frontBackInfo);
    filter.setDistanceFilter(distance);
//    filter.setOcclusionFilter(occlusionFilterLeftCoef, 0);
//    filter.setOcclusionFilter(occlusionFilterRightCoef, 1);
    filter.setOcclusionFilter(occlusionFilterRightCoef);
    
    // the reverb filter
    processorChain.template get<filterIndex>().setHeadShadowFilter(panInfo, frontBackInfo);
    processorChain.template get<filterIndex>().setDistanceFilter(distance);
//    processorChain.template get<filterIndex>().setOcclusionFilter(occlusionFilterLeftCoef, 0);
//    processorChain.template get<filterIndex>().setOcclusionFilter(occlusionFilterRightCoef, 1);
    processorChain.template get<filterIndex>().setOcclusionFilter(occlusionFilterRightCoef);
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
