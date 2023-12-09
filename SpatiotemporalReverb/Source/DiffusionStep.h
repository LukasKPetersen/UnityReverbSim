//
//  DiffusionStep.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 20/10/2023.
//
//  Based on ADC 21' talk: Let's Write a Reverb - Geraint Luff

#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"
#include "Matrix.h"


// we define the structure of the diffusion steps
template <typename Type, size_t numChannels = 8>
class DiffusionStep
{
public:
    DiffusionStep()
    {
    }
    
    void prepare (size_t delayInSamplesUpperBound)
    {
        // we set up each of the diffusion-step channels
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            // we set the delay range for the specific channel
            int lowerBound = std::floor((delayInSamplesUpperBound / numChannels) * ch);
            int upperBound = std::ceil((delayInSamplesUpperBound / numChannels) * (ch + 1));
            juce::Range<int> range(lowerBound, upperBound);


            // we choose a random delay within the sample range
            delayInSamples[ch] = random.nextInt(range);
            
            // we set up the delay line
            delayLines[ch].resize (delayInSamples[ch] + 1);
            delayLines[ch].clear();
            
            // we randomly set polarity inversions
            invertPolarity[ch] = random.nextBool();
        }
    }
    
    void process (Type* input)
    {
        // iterate through the channels writing and reading from the delay line
        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayLines[ch].push(input[ch]);
            input[ch] = delayLines[ch].get(delayInSamples[ch]);
        }
        
        // Mix with a Hadamard matrix
        Hadamard<Type, numChannels>::process (input);
        
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            // TODO: implement a random shuffle (switching the signals between channels)
            
            // invert polarities
            if (invertPolarity[ch]) input[ch] *= -1;
        }
    }
    
private:
    std::array<Type,            numChannels> delayInSamples;
    std::array<DelayLine<Type>, numChannels> delayLines;
    std::array<bool,            numChannels> invertPolarity;
    
    juce::Random random;
};
