//
//  Matrix.h
//  SpatiotemporalReverb
//
//  Created by Lukas Petersen on 20/10/2023.
//

#pragma once
#include <JuceHeader.h>

// Use like `Householder<double, 8>::inPlace(data)` - size must be ≥ 1
//template<typename Type, int size>
//class Householder {
//    static constexpr Type multiplier { -2.0f / size };
//public:
//    static void inPlace (Type *arr) {
//        double sum = 0;
//        for (int i = 0; i < size; ++i)
//            sum += arr[i];
//
//        sum *= multiplier;
//        
//        for (int i = 0; i < size; ++i)
//            arr[i] += sum;
//    };
//};

// Use like `Hadamard<double, 8>::inPlace(data)` - size must be a power of 2
template<typename Type, int size>
class Hadamard {
public:
    static void recursiveMatrixProduct (Type* input) {
        
        if (size <= 1) return;

        // we make two matrices of half the size
        constexpr size_t hSize = size / 2;
        
        Hadamard<Type, hSize>::recursiveMatrixProduct(input);
        Hadamard<Type, hSize>::recursiveMatrixProduct(input + hSize);

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
        Type factor = std::sqrt (1.0f / size);
        
        // looping through each row of the input, corresponding to the diffusion-step channels
        for (int i = 0; i < size; ++i)
            input[i] *= factor;
    }
};
