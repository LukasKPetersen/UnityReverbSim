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
            for (int sample = 0; sample < samples; ++sample)
            {
                auto inputSample = inputBlock.getSample (ch, sample);
                auto filteredSample = inputSample;
                
                // apply head-shadow filter
                filteredSample = headShadowFilter[ch].processSample (filteredSample);
                
                // apply distance filter
                filteredSample = distanceFilter.processSample (filteredSample);
                
                // calculate the output sample
                outputBlock.setSample (ch, sample, dryLevel * inputSample + wetLevel * filteredSample);
            }
        }
    }
    
    void setHeadShadowFilter (float panInfo, float frontBackInfo)
    {
        for (size_t ch = 0; ch < maxNumChannels; ++ch)
        {
            float freqCutoff = 5e3f;
            
            if (frontBackInfo >= 130.0f)
                freqCutoff = 3e2f;
            else if (panInfo > 0.75f && ch == 0)
                freqCutoff = 3e2f;
            else if (panInfo < 0.25f && ch == 1)
                freqCutoff = 3e2f;
            
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
    
    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> headShadowFilter;
    juce::dsp::IIR::Filter<Type> distanceFilter;
    
//    std::vector<juce::dsp::ProcessorDuplicator<FilterType>> filters;
    
//    std::vector<juce::dsp::IIR::Filter<Type>> filters;
//    typename juce::dsp::IIR::Coefficients<Type>::Ptr filterCoefs;
};
