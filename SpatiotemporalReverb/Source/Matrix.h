//
//  Matrix.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 20/10/2023.
//
//  Based on ADC 21' talk: Let's Write a Reverb - Geraint Luff


#pragma once
#include <JuceHeader.h>

/* NOTE: size must be a power of 2 */
template<typename Type, size_t size>
class Hadamard {
public:
    static void recursiveMatrixProduct (Type* input) {
        
        if (size <= 1) return;

        // we make two matrices of half the size
        constexpr size_t hSize = size / 2;
        
        Hadamard<Type, hSize>::recursiveMatrixProduct (input);
        Hadamard<Type, hSize>::recursiveMatrixProduct (input + hSize);

        // we combine the two halves
        for (int i = 0; i < hSize; ++i)
        {
            Type a = input[i];
            Type b = input[i + hSize];
            input[i] = a + b;
            input[i + hSize] = a - b;
        }
    }
    
    static void process(Type* input) {
        recursiveMatrixProduct(input);
        
        Type factor = std::sqrt (1.0 / size);
        
        // looping through each row of the input, corresponding to the diffusion-step channels
        for (int i = 0; i < size; ++i)
            input[i] *= factor;
    }
};
