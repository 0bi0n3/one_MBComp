//
//  basicCompressor.cpp
//  one_MBComp
//
//  Created by Oberon Day-West on 26/07/2023.
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//

#include <JuceHeader.h>
#include "BasicCompressor.h"

void BasicCompressor::prepare( const juce::dsp::ProcessSpec& spec )
{
    spec_ = spec;
    alphaAttack = std::exp(-1.0f / (tauAttack_ * spec.sampleRate / 1000.0f));
    alphaRelease = std::exp(-1.0f / (tauRelease_ * spec.sampleRate / 1000.0f));
}

void BasicCompressor::setThreshold(float newThreshold)
{
    threshold_ = newThreshold;
}

void BasicCompressor::setRatio(float newRatio)
{
    ratio_ = newRatio;
}

void BasicCompressor::setAttackTime(float newAttackTime)
{
    tauAttack_ = newAttackTime;
    alphaAttack = std::exp(-1.0f / (tauAttack_ * spec_.sampleRate / 1000.0f));
}

void BasicCompressor::setReleaseTime(float newReleaseTime)
{
    tauRelease_ = newReleaseTime;
    alphaRelease = std::exp(-1.0f / (tauRelease_ * spec_.sampleRate / 1000.0f));
}

void BasicCompressor::setMakeUpGain(float newMakeUpGain)
{
    makeUpGain_ = newMakeUpGain;
}

void BasicCompressor::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    
    const int numSamples { static_cast<int>(outputBlock.getNumSamples()) };
    const int numChannels { static_cast<int>(outputBlock.getNumChannels()) };

    if( context.isBypassed )
    {
        outputBlock.copyFrom(inputBlock);
    }
    else
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float* channelDataIn = inputBlock.getChannelPointer(channel);
            float* channelDataOut = outputBlock.getChannelPointer(channel);
            
            for (int i = 0; i < numSamples; ++i)
            {
                float sample = channelDataIn[i];
                
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
                
                channelDataOut[i] = sample * c;  // Apply gain to the sample
            }
        }
    }
}


