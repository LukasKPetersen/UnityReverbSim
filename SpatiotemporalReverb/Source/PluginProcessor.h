/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// setting up communication with Unity
#include "/Applications/JUCE/modules/juce_audio_plugin_client/Unity/MyAudioProcessor.h"

// custom reverb functionality
#include "DelayLine.h"
#include "Delay.h"

//==============================================================================
/**
*/
class SpatiotemporalReverbAudioProcessor : public MyAudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SpatiotemporalReverbAudioProcessor();
    ~SpatiotemporalReverbAudioProcessor() override;

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

private:
    // localization parameters
    juce::AudioParameterFloat* gain;
    juce::AudioParameterFloat* pan;
    
    // delay parameters
    juce::AudioParameterFloat* delayTimeLeft;
    juce::AudioParameterFloat* delayTimeRight;
    juce::AudioParameterFloat* wetLevel;
    juce::AudioParameterFloat* dryLevel;
    juce::AudioParameterFloat* feedback;
    juce::AudioParameterFloat* lowPassFreq;
    
    float panSmoother;
    float gainSmoother;
    
    Delay<float> delay;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatiotemporalReverbAudioProcessor)
};
