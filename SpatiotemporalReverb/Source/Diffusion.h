//
//  Diffusion.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 18/10/2023.
//

#pragma once
#include <JuceHeader.h>
#include "DiffusionStep.h"

// based on ADC 21' talk: Let's Write a Reverb - Geraint Luff
template<typename Type, size_t numDiffusionChannels = 8, size_t numDiffusionSteps = 8>
class Diffusion
{
public:
    Diffusion()
    {
        setWetLevel (0.8f);
        setDryLevel (1.0f);
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
                splitSignal.fill(inputSample);
                
                // add the diffusion
                for (size_t step = 0; step < numDiffusionSteps; ++step)
                {
                    if (activeDiffusionSteps[step])
                        splitSignal = diffusionSteps[step].process (splitSignal);
                    else
                        break;
                }
    
                // combine the split signal to a single channel and send it to the output signal
                outputBlock.setSample (ch, sample, std::tanh (std::accumulate (std::begin(splitSignal), std::end(splitSignal), 0.0f)));
            }
        }
    }
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        int samplesPerStep = (int)(diffusionStepAtomicSize * sampleRate);
        
        for (auto& step : diffusionSteps)
        {
            step.prepare (samplesPerStep);
            samplesPerStep *= 2;
        }
        
        // as a default, we only activate the first diffusion step
        activeDiffusionSteps.fill (false);
        activeDiffusionSteps[0] = true;
    }
    
    void adjustDiffusionSize (Type diffusionTime)
    {
        jassert (diffusionTime >= Type (0.0f));
                
        // activate or deactivate diffusion steps, depending on the distance to the listener
        // we always have the first diffusion step activated
        for (int step = 1; step < numDiffusionSteps; ++step)
        {
            activeDiffusionSteps[step] = (diffusionTime > diffusionStepAtomicSize * step) ? true : false;
        }
    }
    
    void setDiffusionStepAtomicSize (Type newSize)
    {
        jassert (newSize > Type (0.0f));
        diffusionStepAtomicSize = newSize;
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
    Type diffusionStepAtomicSize { Type (0.048f) };
    
    std::array<bool, numDiffusionSteps> activeDiffusionSteps;
    
    // we declare an array of diffusion steps that functions as a diffusion chain
    std::array<DiffusionStep<Type, numDiffusionChannels>, numDiffusionSteps> diffusionSteps;
};
