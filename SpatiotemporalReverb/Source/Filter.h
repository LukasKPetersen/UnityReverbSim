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
        setWetLevel (1.0f);
        setDryLevel (0.0f);
    }
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (Type) spec.sampleRate;
        
        filterChain.prepare (spec);
        
        // set the initial cutoff frequencies
        filterChain.template get<distanceFilter>().setCutoffFrequency (10e3f);
        filterChain.template get<occlusionFilter>().setCutoffFrequency (10e3f);
        filterChain.template get<headShadowFilter>().setCutoffFrequency (10e3f);
    }
    
    template <typename ProcessContext>
    void process (const ProcessContext& context)
    {
        filterChain.process (context);
    }
    
    
    void setDistanceFilter (float distance)
    {
        float factor = 10.0f; // for audible effect
        float freqCutoff = 10e3f - distance * factor;
        for (size_t ch = 0; ch < maxNumChannels; ++ch)
            
        filterChain.template get<distanceFilter>().setCutoffFrequency (freqCutoff);
    }
    
    void setOcclusionFilter (float freqCutoff)
    {
        filterChain.template get<occlusionFilter>().setCutoffFrequency (freqCutoff);
    }
    
    void setHeadShadowFilter (float panInfo, float frontBackInfo)
    {
        // a function for calculating the HSF - very simplified (a bit too much, really)
        float freqCutoff = frontBackInfo >= 130.0f ? 5e3f : 10e3f;
        headShadowSmoother -= 0.1 * (headShadowSmoother - freqCutoff); // S-curve applied
        
        filterChain.template get<headShadowFilter>().setCutoffFrequency (headShadowSmoother);
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
    
    void setWetDryBalance (Type newWetLevel)
    {
        // ensure that the input value is valid, i.e. in range [0, 1]
        jassert (newWetLevel >= Type (0) && newWetLevel <= Type (1));
        wetLevel = newWetLevel;
        dryLevel = 1.0f - newWetLevel;
    }

private:
    Type sampleRate { Type (44.1e3) };
    Type wetLevel;
    Type dryLevel;
    Type headShadowSmoother { Type (10e3f) };
    
    // filter chain setup
    enum
    {
        distanceFilter,
        occlusionFilter,
        headShadowFilter
    };
    
    juce::dsp::ProcessorChain<juce::dsp::StateVariableTPTFilter<Type>,
                              juce::dsp::StateVariableTPTFilter<Type>,
                              juce::dsp::StateVariableTPTFilter<Type>> filterChain;
};
