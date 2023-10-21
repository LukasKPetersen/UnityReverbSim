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
        setTotalDiffusionTime(0.120f);
        setWetLevel (0.8f);
        setDryLevel (1.0f);
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
    
    std::vector<Type> processChannelBuffer (size_t channel, const Type* buffer, size_t numSamples)
    {
        std::vector<Type> output(numSamples);
                
        // TODO: make parallel with std::for_each(). Although, I see some potential issues with the delay line...
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            auto inputSample = buffer[sample];
            
            // split the input signal into the diffusion channels
            std::array<Type, numChannels> splitSignal;
            splitSignal.fill(inputSample);
            
            // add the diffusion
            for (auto& step : diffusionSteps)
                splitSignal = step.process (splitSignal);
            
            // combine the split signal to a single channel
            Type outputSample = std::tanh (std::accumulate (std::begin(splitSignal), std::end(splitSignal), 0.0f));
                        
            output[sample] = outputSample;
        }
        
        return output;
    }
    
    void prepare (Type sampleRate)
    {
        this->sampleRate = sampleRate;
        int samplesPerStep = (size_t) 0.040f * sampleRate;
        
        for (int step = 0; step < diffusionSteps.size(); ++step)
        {
            juce::Range<int> diffusionStepRange (samplesPerStep * step, samplesPerStep * (step + 1));
            diffusionSteps[step].prepare (diffusionStepRange);
        }
    }
    
    void setTotalDiffusionTime (Type newDiffusionTime)
    {
        jassert (newDiffusionTime > Type (0.0f));
        
        totalDiffusionTime = newDiffusionTime;
        
        // we adjust the number of diffusion steps so that there is one step for every 40ms
        while (totalDiffusionTime / Type (0.040f) < diffusionSteps.size())
            diffusionSteps.push_back(DiffusionStep<Type, numChannels>());
        
        while (totalDiffusionTime / Type (0.040f) > diffusionSteps.size())
            diffusionSteps.pop_back();
        
        prepare(sampleRate);
    }
    
//    void setDiffusionStepLengths (float totalDiffusionTimeInSeconds)
//    {
//        for (auto& step : diffusionSteps)
//            step.delayRangeInSeconds = totalDiffusionTimeInSeconds / ;
//    }
    
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
    Type totalDiffusionTime { Type (0.120f) };
    
    // we declare an array of diffusion steps that functions as a diffusion chain
//    std::array<DiffusionStep<Type, numChannels>, stepCount> diffusionSteps;
    std::vector<DiffusionStep<Type, numChannels>> diffusionSteps;
};
