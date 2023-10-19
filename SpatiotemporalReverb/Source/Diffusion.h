//
//  Diffusion.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 18/10/2023.
//

#pragma once
#include <JuceHeader.h>

// based on ADC 21' talk: Let's Write a Reverb - Geraint Luff

template<typename Type, size_t numChannels = 8, size_t stepCount = 4>
class Diffusion
{
public:
    Diffusion(float totalDiffusionTime)
    {
        setDiffusionStepLengths(totalDiffusionTime);
    }
    
    std::vector<Type> process (std::vector<Type> input)
    {
        for (auto& step : diffusionSteps)
            input = step.process (input);
        
        return input;
    }
    
    void configure (double sampleRate)
    {
        for (auto& step : steps)
            step.configure (sampleRate);
    }
    
    void setDiffusionStepLengths (float totalDiffusionTime)
    {
        for (auto& step : diffusionSteps)
            step.delayMsRange = totalDiffusionTime / stepCount;
    }
    
private:
    std::array<DiffusionStep<Type>, stepCount> diffusionSteps;

    
    template<typename Type, size_t numChannels = 8>
    struct DiffusionStep
    {
        using Array = std::array<double, numChannels>;
        double delayMsRange = 50;
        
        std::array<int, numChannels> delaySamples;
        std::array<Delay, numChannels> delays; // delay lines
        std::array<bool, numChannels> flipPolarity;
        
        void configure (double sampleRate)
        {
            double delaySamplesRange = delayMsRange * 0.001 * sampleRate;
            for (int c = 0; c < numChannels; ++c)
            {
                double rangeLow = delaySamplesRange * c / numChannels;
                double rangeHigh = delaySamplesRange * (c + 1) / numChannels;
                delaySamples[c] = randomInRange (rangeLow, rangeHigh);
                delays[c].resize (delaySamples[c] + 1);
                delays[c].reset();
                flipPolarity[c] = rand() % 2;
            }
        }
        
        Array process (Array input)
        {
            // Delay
            Array delayed;
            for (int c = 0; c < numChannels; ++c)
            {
                delays[c].write(input[c]);
                delayed[c] = delays[c].read(delaySamples[c]);
            }
            
            // Flip some polarities
            for (int c = 0; c < numChannels; ++c)
            {
                if (flipPolarity[c]) delayed[c] *= -1;
            }
            
            // Mix with a Hadamard matrix
            Array mixed = delayed;
            Hadamard<double, numChannels>::inPlace(mixed.data());
            
            return mixed;
        }
    };
};
