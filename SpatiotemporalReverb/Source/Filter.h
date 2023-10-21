//
//  Filter.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 21/10/2023.
//

#pragma once
#include <JuceHeader.h>

template <typename Type, size_t maxNumChannels = 2>
class Filter
{
public:
    Filter()
    {
        setWetLevel (0.8f);
        setDryLevel (1.0f);
        
        headShadowFreqSmoothingLeft = 5e3f;
        headShadowFreqSmoothingRight = 5e3f;
    }
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (Type) spec.sampleRate;
        
        // set up the basic head shadow filter
        for (auto& filter : headShadowFilter)
        {
            filter.prepare (spec);
            filter.coefficients = juce::dsp::IIR::Coefficients<Type>::makeAllPass (sampleRate, 5e3f);
        }
        
        // set up the distance filter
        distanceFilter.prepare (spec);
        distanceFilter.coefficients = juce::dsp::IIR::Coefficients<Type>::makeAllPass (sampleRate, 5e3f);
        
//        for (auto& filter : filters)
//            filter.prepare (spec);
    }

    std::vector<Type> processChannelBuffer (size_t channel, const Type* buffer, size_t numSamples)
    {
        std::vector<Type> output(numSamples);
                
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            auto inputSample = buffer[sample];
            auto filteredSample = inputSample;
            
            
            // apply filters to sample
//            for (auto& filter : filters)
//                filteredSample = filter.processSample (filteredSample);
            
            // apply head-shadow filter
            filteredSample = headShadowFilter[channel].processSample (filteredSample);
            
            // apply distance filter
            filteredSample = distanceFilter.processSample (filteredSample);
            
            // calculate the output sample
            auto outputSample = dryLevel * inputSample + wetLevel * filteredSample;
            output[sample] = outputSample;
        }
        
        return output;
    }
    
    void setHeadShadowFilter (float panInfo, float frontBackInfo)
    {
        for (size_t ch = 0; ch < maxNumChannels; ++ch)
        {
            float freqCutoff = 5e3f;
            
            if (frontBackInfo >= 130.0f)
                freqCutoff = 3e3f;
            else if (panInfo > 0.75f && ch == 0)
                freqCutoff = 3e3f;
            else if (panInfo < 0.25f && ch == 1)
                freqCutoff = 3e3f;
            
            headShadowFilter[ch].coefficients = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass(sampleRate, freqCutoff);
        }
    }
    
    void setDistanceFilter (float distance)
    {
        float freqCutoff = 5e3f - distance * 10.0f; // this factor is based on gut feeling (set it quite high for audiable effect)
        distanceFilter.coefficients = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass(sampleRate, freqCutoff);
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

private:
    Type sampleRate { Type (44.1e3) };
    Type wetLevel;
    Type dryLevel;
    
    float headShadowFreqSmoothingLeft;
    float headShadowFreqSmoothingRight;
    
    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> headShadowFilter;
    juce::dsp::IIR::Filter<Type> distanceFilter;
    
//    std::vector<juce::dsp::ProcessorDuplicator<FilterType>> filters;
    
//    std::vector<juce::dsp::IIR::Filter<Type>> filters;
//    typename juce::dsp::IIR::Coefficients<Type>::Ptr filterCoefs;
};
