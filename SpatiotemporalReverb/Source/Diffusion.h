//
//  Diffusion.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 18/10/2023.
//
//  Based on ADC 21' talk: Let's Write a Reverb - Geraint Luff


#pragma once
#include <JuceHeader.h>
#include "DiffusionStep.h"

template<typename Type, size_t numDiffusionChannels = 8, size_t numDiffusionSteps = 8>
class Diffusion
{
public:
    Diffusion()
    {
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
                
                // split the input signal into the diffusion channels
                std::array<Type, numDiffusionChannels> splitSignal;
                splitSignal.fill (inputSample);
                
                // add the diffusion
                for (size_t step = 0; step <= activeDiffusionSteps; ++step)
                    diffusionSteps[step].process (splitSignal.data());
    
                // combine the split signal to a single channel and send it to the output signal
                auto outputSample = (std::accumulate (std::begin(splitSignal), std::end(splitSignal), 0.0f)) / numDiffusionChannels;
                outputBlock.setSample (ch, sample, outputSample);
            }
        }
    }
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        
        // make sure that the diffusion step size is proportional to the sample rate
        jassert (diffusionStepAtomicSize * sampleRate > 1);
        
        size_t samplesPerStep = (size_t)(diffusionStepAtomicSize * sampleRate);
        
        for (auto& step : diffusionSteps)
        {
            step.prepare (samplesPerStep);
            samplesPerStep *= 2; // for every step we double the diffusion length
        }
    }
    
    void setDiffusionStep (float diffusionTime)
    {
        // apply S-curve to make sure we don't jump multiple steps at once
        diffusionTimeSmoother -= 0.02 * (diffusionTimeSmoother - diffusionTime);
        
        // calculate the amount of diffusion steps needed
        int step = 0;
        while (diffusionTimeSmoother > diffusionStepAtomicSize * (2 << step) && step <= numDiffusionSteps) step++;
        activeDiffusionSteps = step;
    }
    
private:
    Type sampleRate { Type (44.1e3) };
    Type diffusionStepAtomicSize { Type (0.012f) };
    
    size_t activeDiffusionSteps { 0 };
    Type diffusionTimeSmoother { Type (0.24f) };
    
    // we declare an array of diffusion steps that functions as a diffusion chain
    std::array<DiffusionStep<Type, numDiffusionChannels>, numDiffusionSteps> diffusionSteps;
};
