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
template<typename Type, size_t numChannels = 8, size_t stepCount = 4>
class Diffusion
{
public:
    Diffusion()
    {
    }
    
    Type processSample (Type input)
    {
        // split the input signal into the diffusion channels
        std::array<Type, numChannels> splitSignal;
        splitSignal.fill(input);
        
        for (auto& step : diffusionSteps)
            splitSignal = step.process (splitSignal);
        
        // combine the split signal to a single channel
        Type result = std::tanh (std::accumulate (std::begin(splitSignal), std::end(splitSignal), 0.0f));
        
        return result;
    }
    
    void prepare (double sampleRate)
    {
        for (auto& step : diffusionSteps)
            step.prepare (sampleRate);
    }
    
    void setDiffusionStepLengths (float totalDiffusionTimeInSeconds)
    {
        for (auto& step : diffusionSteps)
            step.delayRangeInSeconds = totalDiffusionTimeInSeconds / stepCount;
    }
    
private:
    // we declare an array of diffusion steps that functions as a diffusion chain
    std::array<DiffusionStep<Type, numChannels>, stepCount> diffusionSteps;
};
