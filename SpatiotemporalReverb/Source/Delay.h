//
//  Delay.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 01/10/2023.
//

#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"
#include "Filter.h"
#include <memory>

template <typename Type, size_t maxNumChannels = 2>
class Delay
{
public:
    Delay()
    {
        setMaxDelayTime (5.0f);
        setDelayTime (0, 0.5f); // set left channel delay
        setDelayTime (1, 0.5f); // set right channel delay
        setWetLevel (0.8f);
        setDryLevel (1.0f);
        setFeedback (0.0f); // I don't think I'll need feedback but it's easier to remove than add...
    }
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        // ensure that the input is valid
        jassert (spec.numChannels <= maxNumChannels);
        
        sampleRate = (Type) spec.sampleRate;
        updateDelayLineSize();
    }
    
    void reset()
    {
        for (auto& delayLine : delayLines)
            delayLine.clear();
    }
    
    template <typename ProcessContext>
    void process (const ProcessContext& context)
    {
        auto inputBlock = context.getInputBlock();
        auto outputBlock = context.getOutputBlock();
        
        size_t channels = inputBlock.getNumChannels();
        size_t samples = inputBlock.getNumSamples();
        
        for (int ch = 0; ch < channels; ++ch)
        {
            auto& delayLine = delayLines[ch];
            
            for (int sample = 0; sample < samples; ++sample)
            {
                auto inputSample = inputBlock.getSample (ch, sample);
                                        
                // get the delayedSignal from the delay line and apply filter
                auto delayedSample = delayLine.get(delayTimes[ch] * sampleRate);
                
                // add diffused echo to delay line
                delayLine.push(inputSample + feedback * delayedSample);
                
                // calculate the output sample and send it to the output signal
                outputBlock.setSample (ch, sample, dryLevel * inputSample + wetLevel * delayedSample);
            }
        }
    }
    
    
//    void clearEchoes ()
//    {
//        for (std::vector<Echo>& echoesVector : echoes)
//            echoesVector.clear();
//    }
//    
//    void addEcho (float delayTime, float soundReduction, int channel)
//    {
//        echoes[channel].push_back(Echo {delayTime, soundReduction});
//    }
    
    void setMaxDelayTime (Type newMax)
    {
        // we ensure that the input value is valid
        jassert (newMax > Type (0));
        maxDelayTime = newMax;
        
        updateDelayLineSize();
    }
    
    void setDelayTime (size_t channel, Type newDelayTime)
    {
        // ensure that the input values are valid
        jassert (channel <= maxNumChannels);
        jassert (newDelayTime >= Type (0) && newDelayTime <= maxDelayTime);
        
        delayTimes[channel] = newDelayTime;
    }
    
    void setWetLevel (Type newWetLevel)
    {
        // ensure that the input value is valid, i.e. in range [0, 1]
        jassert (newWetLevel >= Type (0) && newWetLevel <= Type (1));
        wetLevel = newWetLevel;
    }
    
    void setDryLevel (Type newDryLevel)
    {
        // ensure that the input value is valid, i.e. in range [0, 1]
        jassert (newDryLevel >= Type (0) && newDryLevel <= Type (1));
        dryLevel = newDryLevel;
    }
    
    void setFeedback (Type newFeedbackValue)
    {
        // ensure that the input value is valid, i.e. in range [0, 1]
        jassert (newFeedbackValue >= Type (0) && newFeedbackValue <= Type (1));
        feedback = newFeedbackValue;
    }

private:
    // parameters
    Type maxDelayTime { 2.0f };
    Type sampleRate { Type (44.1e3) };
    Type wetLevel;
    Type dryLevel;
    Type feedback;
    
    juce::Random random;
    
    // delay lines
    std::array<DelayLine<Type>, maxNumChannels> delayLines;
    std::array<Type,            maxNumChannels> delayTimes;
    
    // we define the data structure that holds each individual echo
//    struct Echo
//    {
//        float delayInSeconds;
//        float soundReduction;
//    };
//    std::array<std::vector<Echo>, maxNumChannels> echoes;

    // filters
//    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> filters;
//    typename juce::dsp::IIR::Coefficients<Type>::Ptr filterCoefs;
    
    // helper function
    void updateDelayLineSize()
    {
        // we adjust the size of the circular buffers for each of our delay lines
        size_t delayLineSamples = (size_t) std::ceil(maxDelayTime * sampleRate);
        for (auto& delayLine : delayLines)
            delayLine.resize (delayLineSamples);
    }
};
