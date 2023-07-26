//
//  basicCompressor.cpp
//  one_MBComp
//
//  Created by Oberon Day-West on 26/07/2023.
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//

#include <JuceHeader.h>
#include "BasicCompressor.h"

void BasicCompressor::prepare(double sampleRate)
{
    alphaAttack = std::exp(-1.0f / (tauAttack_ * sampleRate / 1000.0f));
    alphaRelease = std::exp(-1.0f / (tauRelease_ * sampleRate / 1000.0f));
}

void BasicCompressor::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int i = 0; i < numSamples; ++i)
        {
            float& sample = channelData[i];

            float x_g;
            if (std::abs(sample) < 0.000001f)
                x_g = -120.0f;
            else
                x_g = 20.0f * std::log10(std::abs(sample));

            float y_g;
            if (x_g >= threshold_)
                y_g = threshold_ + (x_g - threshold_) / ratio_;
            else
                y_g = x_g;

            float x_l = x_g - y_g;

            float y_l;
            if (x_l > yL_prev)
                y_l = alphaAttack * yL_prev + (1.0f - alphaAttack) * x_l;
            else
                y_l = alphaRelease * yL_prev + (1.0f - alphaRelease) * x_l;

            float c = std::pow(10.0f, (makeUpGain_ - y_l) / 20.0f);
            yL_prev = y_l;

            sample *= c;  // Apply gain to the sample
        }
    }
}

