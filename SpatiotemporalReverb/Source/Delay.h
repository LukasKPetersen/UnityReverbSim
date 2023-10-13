//
//  Delay.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 01/10/2023.
//

#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"
#include <memory>

template <typename Type, size_t maxNumChannels = 2>
class Delay
{
public:
    Delay()
    {
        setMaxDelayTime (2.0f);
        setDelayTime (0, 0.5f); // set left channel delay
        setDelayTime (1, 0.5f); // set right channel delay
        setWetLevel (0.8f);
        setDryLevel (1.0f);
        setFeedback (0.5f); // I don't think I'll need feedback but it's easier to remove than add...
    }
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        // ensure that the input is valid
        jassert (spec.numChannels <= maxNumChannels);
        sampleRate = (Type) spec.sampleRate;
        updateDelayLineSize();
        
        filterCoefs = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass(sampleRate, Type (1e2));
        
        for (auto& filter : filters)
        {
            filter.prepare (spec);
            filter.coefficients = filterCoefs;
        }
    }
    
    void setLowpassFreq (size_t channel, float freq)
    {
        filters[channel].coefficients = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass(sampleRate, freq);
    }
    
    void reset()
    {
        for (auto& filter : filters)
            filter.reset();
        
        for (auto& delayLine : delayLines)
            delayLine.clear();
    }
    
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
    
    std::vector<float> processChannelBuffer (size_t channel, const Type* buffer, size_t numSamples)
    {
        std::vector<float> output(numSamples);
        
        auto& delayLine = delayLines[channel];
        auto delayTime = delayTimes[channel] * sampleRate; // calculate the delay time in samples
        auto& filter = filters[channel];
        
        // TODO: make parallel with std::for_each(). Although, I see some potential issues with the delay line...
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            // read the delayed sample from the delay line
            auto delayedSample = filter.processSample (delayLine.get(delayTime));
            
            // push the current input sample to the delay line (to be read back after delay time has passed)
            auto inputSample = buffer[sample];
            auto delayLineInputSample = std::tanh(inputSample + feedback * delayedSample);
            delayLine.push(delayLineInputSample);
            
            // calculate the output sample
            auto outputSample = dryLevel * inputSample + wetLevel * delayedSample;
            output[sample] = outputSample;
        }
        
        return output;
    }
    
private:
    // fields (initialization necessary)
    Type maxDelayTime { 2.0f };
    Type sampleRate { Type (44.1e3) };
    Type wetLevel;
    Type dryLevel;
    Type feedback;
    
    // delay lines
    std::array<DelayLine<Type>, maxNumChannels> delayLines;
    std::array<Type,            maxNumChannels> delayTimes;

    // filters
    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> filters;
    typename juce::dsp::IIR::Coefficients<Type>::Ptr filterCoefs;
    
    // helper function
    void updateDelayLineSize()
    {
        // we adjust the size of the circular buffers for each of our delay lines
        size_t delayLineSamples = (size_t) std::ceil(maxDelayTime * sampleRate);
        for (auto& delayLine : delayLines)
            delayLine.resize (delayLineSamples);
    }
};
