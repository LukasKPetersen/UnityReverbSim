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
        filterChain.template get<distanceFilter>().setCutoffFrequency (5e3f);
        filterChain.template get<occlusionFilter>().setCutoffFrequency (5e3f);
        filterChain.template get<headShadowFilter>().setCutoffFrequency (5e3f);
        
        // set up the distance filter
//        distanceFilter.prepare (spec);
//        distanceFilter.setCutoffFrequency (5e3f);
//        for (auto& filter : distanceFilter)
//        {
//            filter.prepare (spec);
//            filter.coefficients = juce::dsp::IIR::Coefficients<Type>::makeAllPass (sampleRate, 5e3f);
//        }
        
        // set up occlusion filter
//        occlusionFilter.prepare (spec);
//        occlusionFilter.setCutoffFrequency (5e3f);
//        for (auto& filter : occlusionFilter)
//        {
//            filter.prepare (spec);
//            filter.coefficients = juce::dsp::IIR::Coefficients<Type>::makeAllPass (sampleRate, 5e3f);
//        }
        
        // set up the basic head shadow filter
//        headShadowFilter.prepare (spec);
//        headShadowFilter.setCutoffFrequency (5e3f);
//        for (auto& filter : headShadowFilter)
//        {
//            filter.prepare (spec);
//            filter.coefficients = juce::dsp::IIR::Coefficients<Type>::makeAllPass (sampleRate, 5e3f);
//        }
    }
    
    template <typename ProcessContext>
    void process (const ProcessContext& context)
    {
        
        filterChain.process (context);
        
//        distanceFilter.process (context);
//        occlusionFilter.process (context);
//        headShadowFilter.process (context);
        
//        auto inputBlock = context.getInputBlock();
//        auto outputBlock = context.getOutputBlock();
//        
//        size_t channels = inputBlock.getNumChannels();
//        size_t samples = inputBlock.getNumSamples();
//        
//        for (int ch = 0; ch < channels; ++ch)
//        {
//            for (int sample = 0; sample < samples; ++sample)
//            {
//                auto inputSample = inputBlock.getSample (ch, sample);
//                auto filteredSample = inputSample;
//                
//                // apply distance filter
//                filteredSample = distanceFilter[ch].processSample (filteredSample);
//                
//                // apply occlusion filter
//                filteredSample = occlusionFilter[ch].processSample (filteredSample);
//                
//                // apply head-shadow filter
//                filteredSample = headShadowFilter[ch].processSample (filteredSample);
//                
//                // calculate the output sample
//                auto outputSample = std::tanh (dryLevel * inputSample + wetLevel * filteredSample);
//                outputBlock.setSample (ch, sample, outputSample);
//            }
//            
////            distanceFilter.snapToZero();
////            occlusionFilter[ch].snapToZero();
////            headShadowFilter[ch].snapToZero();
//        }
    }
    
//    void setDistanceFilter (float distance)
//    {
//        float factor = 10.0f; // for audible effect
//        float freqCutoff = 5e3f - distance * factor;
//        for (size_t ch = 0; ch < maxNumChannels; ++ch)
//            
//        distanceFilter.setCutoffFrequency (freqCutoff);
//            
////        {
////            distanceFilter[ch].coefficients = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass(sampleRate, freqCutoff);
////        }
////        
////        distanceFilter.reset();
//    }
    
    void setDistanceFilter (float distance)
    {
        float factor = 10.0f; // for audible effect
        float freqCutoff = 5e3f - distance * factor;
        for (size_t ch = 0; ch < maxNumChannels; ++ch)
            
        filterChain.template get<distanceFilter>().setCutoffFrequency (freqCutoff);
    }
    
//    void setOcclusionFilter (float freqCutoff, int ch)
//    {
//        occlusionFilter[ch].coefficients = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass(sampleRate, freqCutoff);
//        
////        occlusionFilter[channel].reset();
//    }
    
//    void setOcclusionFilter (float freqCutoff)
//    {
//        occlusionFilter.setCutoffFrequency (freqCutoff);
//    }
    
    void setOcclusionFilter (float freqCutoff)
    {
        filterChain.template get<occlusionFilter>().setCutoffFrequency (freqCutoff);
    }
    
//    void setHeadShadowFilter (float panInfo, float frontBackInfo)
//    {
//        float freqCutoff = 5e3f;
//        if (frontBackInfo >= 130.0f)
//            freqCutoff = 3e2f;
//        
//        headShadowFilter.setCutoffFrequency (freqCutoff);
//        
//        
//        // TODO: make a more sophisticated function here
////        for (size_t ch = 0; ch < maxNumChannels; ++ch)
////        {
////            float freqCutoff = 5e3f;
////            
////            if (frontBackInfo >= 130.0f)
////                freqCutoff = 3e2f;
////            else if (panInfo > 0.75f && ch == 0)
////                freqCutoff = 3e2f;
////            else if (panInfo < 0.25f && ch == 1)
////                freqCutoff = 3e2f;
////            
////            headShadowFilter[ch].coefficients = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass(sampleRate, freqCutoff);
//////            headShadowFilter[ch].reset();
////        }
//    }
    
//    void setHeadShadowFilter (float panInfo, float frontBackInfo)
//    {
//        float freqCutoff = 5e3f;
//        if (frontBackInfo >= 130.0f)
//            freqCutoff = 3e2f;
//        
//        headShadowFilter.setCutoffFrequency (freqCutoff);
//    }
    
    void setHeadShadowFilter (float panInfo, float frontBackInfo)
    {
        float freqCutoff = 5e3f;
        if (frontBackInfo >= 130.0f)
            freqCutoff = 3e2f;
        
        filterChain.template get<headShadowFilter>().setCutoffFrequency (freqCutoff);
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
    
//    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> distanceFilter;
//    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> occlusionFilter;
//    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> headShadowFilter;
    
//    juce::dsp::StateVariableTPTFilter<Type> distanceFilter;
//    juce::dsp::StateVariableTPTFilter<Type> occlusionFilter;
//    juce::dsp::StateVariableTPTFilter<Type> headShadowFilter;
    
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
