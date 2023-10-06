//
//  DelayLine.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 01/10/2023.
//

#pragma once
#include <JuceHeader.h>

template <typename Type>
class DelayLine
{
public:
    void clear()
    {
        std::fill (rawData.begin(), rawData.end(), Type (0));
    }
    
    size_t getSize() const
    {
        return rawData.size();
    }
    
    void push (Type valueToAdd) noexcept
    {
        rawData[nextIndex] = valueToAdd;
        
        // wrap around the buffer if the nextIndex is at the end
        nextIndex = nextIndex == 0 ? getSize() - 1 : nextIndex - 1;
    }
    
    Type get (size_t delayInSamples) const noexcept
    {
        // make sure that delayInSamples is within the bounds
        jassert ((delayInSamples >= 0) && (delayInSamples < getSize()));

        // we wrap around the buffer if the index exceeds the size of the buffer
        return rawData[(nextIndex + 1 + delayInSamples) % getSize()];
    }
    
    void setSample (size_t delayInSamples, Type newValue) noexcept
    {
        // make sure that delayInSamples is within the bounds
        jassert ((delayInSamples >= 0) && (delayInSamples < getSize()));
        
        // we wrap around the buffer if the index exceeds the size of the buffer
        rawData[(nextIndex + 1 + delayInSamples) % getSize()] = newValue;
    }
    
    void resize (size_t numSamples)
    {
        // ensure that the input value is valid
        jassert (numSamples > 0);
        rawData.resize (numSamples);
        nextIndex = 0;
    }

private:
    std::vector<Type> rawData;
    size_t nextIndex = 0;
};
