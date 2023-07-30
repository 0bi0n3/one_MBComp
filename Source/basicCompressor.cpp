//
//  basicCompressor.cpp
//  one_MBComp
//
//  UWL #21501990.
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//  This code has been referenced and adapted from Reiss and McPherson (2015), Pirkle (2019) and Tarr (2019).
//  Please refer to the accompanying report for full list of references.

#include <JuceHeader.h>
#include "BasicCompressor.h"

void BasicCompressor::prepare( const juce::dsp::ProcessSpec& spec )
{
    spec_ = spec;
    alphaAttack = std::exp(-1.0f / (tauAttack_ * spec.sampleRate / 1000.0f));
    alphaRelease = std::exp(-1.0f / (tauRelease_ * spec.sampleRate / 1000.0f));
}

void BasicCompressor::setThresholdLevel(float newThreshold)
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
    // Get input and output blocks from the context
    auto& inputAudioBlock = context.getInputBlock();
    auto& outputAudioBlock = context.getOutputBlock();
    
    // Get the number of samples and channels from the output block
    const int numberOfSamples { static_cast<int>(outputAudioBlock.getNumSamples()) };
    const int numberOfChannels { static_cast<int>(outputAudioBlock.getNumChannels()) };

    // If context is bypassed, copy input block to output block without processing
    if(context.isBypassed)
    {
        outputAudioBlock.copyFrom(inputAudioBlock);
    }
    else
    {
        // If not bypassed, iterate over each channel
        for (int currentChannel = 0; currentChannel < numberOfChannels; ++currentChannel)
        {
            // Get the data for current input and output channels
            const float* inputChannelData = inputAudioBlock.getChannelPointer(currentChannel);
            float* outputChannelData = outputAudioBlock.getChannelPointer(currentChannel);
            
            // Iterate over each sample in the channel
            for (int currentSampleIndex = 0; currentSampleIndex < numberOfSamples; ++currentSampleIndex)
            {
                // Get the current sample
                float currentSample = inputChannelData[currentSampleIndex];
                
                // Compute the input level in decibels
                float inputLevelInDecibels;
                if (std::abs(currentSample) < 0.000001f)
                    inputLevelInDecibels = -120.0f;
                else
                    inputLevelInDecibels = 20.0f * std::log10(std::abs(currentSample));
                
                // Compute the output level in decibels
                float outputLevelInDecibels;
                if (inputLevelInDecibels >= threshold_)
                    outputLevelInDecibels = threshold_ + (inputLevelInDecibels - threshold_) / ratio_;
                else
                    outputLevelInDecibels = inputLevelInDecibels;
                
                // Compute the difference between input and output levels
                float levelDifference = inputLevelInDecibels - outputLevelInDecibels;
                
                // Apply attack or release envelope to the level difference
                float envelopeLevel;
                if (levelDifference > previousEnvelopeLevel)
                    envelopeLevel = alphaAttack * previousEnvelopeLevel + (1.0f - alphaAttack) * levelDifference;
                else
                    envelopeLevel = alphaRelease * previousEnvelopeLevel + (1.0f - alphaRelease) * levelDifference;
                
                // Compute the gain to be applied on the sample
                float gainForSample = std::pow(10.0f, (makeUpGain_ - envelopeLevel) / 20.0f);
                previousEnvelopeLevel = envelopeLevel;
                
                // Apply gain to the sample and write it to the output
                outputChannelData[currentSampleIndex] = currentSample * gainForSample;
            }
        }
    }
}



