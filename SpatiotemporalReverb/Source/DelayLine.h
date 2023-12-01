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
    
    // TODO: Deprecated - delete asap
    void push (Type valueToAdd) noexcept
    {
        rawData[writeIndex] = valueToAdd;
        
        // wrap around the buffer if the writeIndex is at the end
        writeIndex = writeIndex == 0 ? getSize() - 1 : writeIndex - 1;
    }
    
    // TODO: Deprecated - delete asap
    Type get (size_t delayInSamples) const noexcept
    {
        // make sure that delayInSamples is within the bounds
        jassert ((delayInSamples >= 0) && (delayInSamples < getSize()));

        // we wrap around the buffer if the index exceeds the size of the buffer
        return rawData[(writeIndex + 1 + delayInSamples) % getSize()];
    }
    
    void setSample (size_t delayInSamples, Type newValue) noexcept
    {
        // make sure that delayInSamples is within the bounds
        jassert ((delayInSamples >= 0) && (delayInSamples < getSize()));
        
        // we wrap around the buffer if the index exceeds the size of the buffer
        rawData[(writeIndex + 1 + delayInSamples) % getSize()] = newValue;
    }
    
    void addSample (size_t delayInSamples, Type newSample) noexcept
    {
        // make sure that delayInSamples is within the bounds
        jassert ((delayInSamples >= 0) && (delayInSamples < getSize()));
        
        auto existingSample = rawData[(readIndex + 1 + delayInSamples) % getSize()];
        
        // we wrap around the buffer if the index exceeds the size of the buffer
        // we use a tangent hyperbolic function to make a clean accumulated sample
        rawData[(readIndex + 1 + delayInSamples) % getSize()] = std::tanh(existingSample + newSample);
    }
    
    Type getNextSample ()
    {
        Type nextSample = rawData[(readIndex + 1) % getSize()];
        // we make sure to reset the data after we read it
        rawData[(readIndex + 1) % getSize()] = Type (0);
        
        // wrap around the buffer if the readIndex is at the end
        readIndex = readIndex == getSize() - 1 ? 0 : readIndex + 1;
        
        return nextSample;
    }
    
    void resize (size_t numSamples)
    {
        // ensure that the input value is valid
        jassert (numSamples > 0);
        rawData.resize (numSamples);
        writeIndex = 0;
        clear();
    }

private:
    std::vector<Type> rawData;
    size_t writeIndex = 0; // TODO: Deprecated - delete asap
    size_t readIndex = 0;
};
