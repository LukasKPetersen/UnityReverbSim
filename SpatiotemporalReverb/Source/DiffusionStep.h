//
//  DiffusionStep.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 20/10/2023.
//

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
    
    void prepare (juce::Range<int> delaySamplesRange)
    {
        // we set up each of the diffusion-step channels
        for (int ch = 0; ch < numChannels; ++ch)
        {
            // we set the delay range for the specific channel
            juce::Range<int> range((delaySamplesRange.getLength() / numChannels) * ch, (delaySamplesRange.getLength() / numChannels) * (ch + 1));
            // we choose a random delay within the range
            delayInSamples[ch] = random.nextInt(delaySamplesRange) + delaySamplesRange.getStart();
            
            // we set up the delay line
            delayLines[ch].resize (delayInSamples[ch] + 1);
            delayLines[ch].clear();
            
            // we randomly set polarity inversions
            invertPolarity[ch] = random.nextBool();
        }
    }
    
    std::array<Type, numChannels> process (std::array<Type, numChannels> input)
    {
        std::array<Type, numChannels> delayed;
        
        // iterate through the channels, writing to the delay line,
        // reading from the delay line, and inverting polarities
        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayLines[ch].push(input[ch]);
            delayed[ch] = delayLines[ch].get(delayInSamples[ch]);
            
            // invert polarities
            if (invertPolarity[ch]) delayed[ch] *= -1;
        }
        
        // Mix with a Hadamard matrix
        std::array<Type, numChannels> hadamardProduct = delayed;
        Hadamard<Type, numChannels>::process(hadamardProduct.data());

        return hadamardProduct;
    }
    
    void setRange (float delayInSeconds)
    {
        delayRangeInSeconds = delayInSeconds;
    }
    
private:
    float delayRangeInSeconds { 0.050f };
    
    std::array<size_t, numChannels> delayInSamples;
    std::array<DelayLine<Type>, numChannels> delayLines;
    std::array<bool, numChannels> invertPolarity;
    
    juce::Random random;
};
