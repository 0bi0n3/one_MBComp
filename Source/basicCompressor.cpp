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

// This method prepares the compressor with provided specifications
void BasicCompressor::prepare( const juce::dsp::ProcessSpec& compressorSpec )
{
    // Copy the provided specification to the class's specification member
    m_compressorSpecifications = compressorSpec;

    // Compute the alpha value for attack phase using provided specifications
    m_alphaAttack = std::exp(-1.0f / (m_releaseTimeInMs * m_compressorSpecifications.sampleRate / 1000.0f));
    
    // Compute the alpha value for release phase using provided specifications
    m_alphaRelease = std::exp(-1.0f / (m_releaseTimeInMs * m_compressorSpecifications.sampleRate / 1000.0f));
}

// This method sets the threshold level of the compressor
void BasicCompressor::setThresholdLevel(float newThresholdLevel)
{
    m_thresholdLevelDb = newThresholdLevel;
}

// This method sets the compression ratio of the compressor
void BasicCompressor::setCompressionRatio(float newCompressionRatio)
{
    m_compressionRatio = newCompressionRatio;
}

// This method sets the attack time of the compressor
void BasicCompressor::setAttackTime(float newAttackTimeInMs)
{
    m_attackTimeInMs = newAttackTimeInMs;

    // Compute the new alpha value for attack phase after attack time is updated
    m_alphaAttack = std::exp(-1.0f / (m_attackTimeInMs * m_compressorSpecifications.sampleRate / 1000.0f));
}

// This method sets the release time of the compressor
void BasicCompressor::setReleaseTime(float newReleaseTimeInMs)
{
    m_releaseTimeInMs = newReleaseTimeInMs;

    // Compute the new alpha value for release phase after release time is updated
    m_alphaRelease = std::exp(-1.0f / (m_releaseTimeInMs * m_compressorSpecifications.sampleRate / 1000.0f));
}

// This method sets the make-up gain of the compressor
void BasicCompressor::setMakeUpGain(float newMakeUpGain)
{
    m_newMakeUpGainDb = newMakeUpGain;
}

// Main compressor processing method
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
                if (inputLevelInDecibels >= m_thresholdLevelDb)
                    outputLevelInDecibels = m_thresholdLevelDb + (inputLevelInDecibels - m_thresholdLevelDb) / m_compressionRatio;
                else
                    outputLevelInDecibels = inputLevelInDecibels;
                
                // Compute the difference between input and output levels
                float levelDifference = inputLevelInDecibels - outputLevelInDecibels;
                
                // Apply attack or release envelope to the level difference
                float envelopeLevel;
                if (levelDifference > m_previousEnvelopeLevel)
                    envelopeLevel = m_alphaAttack * m_previousEnvelopeLevel + (1.0f - m_alphaAttack) * levelDifference;
                else
                    envelopeLevel = m_alphaRelease * m_previousEnvelopeLevel + (1.0f - m_alphaRelease) * levelDifference;
                
                // Compute the gain to be applied on the sample
                float gainForSample = std::pow(10.0f, (m_newMakeUpGainDb - envelopeLevel) / 20.0f);
                m_previousEnvelopeLevel = envelopeLevel;
                
                // Apply gain to the sample and write it to the output
                outputChannelData[currentSampleIndex] = currentSample * gainForSample;
            }
        }
    }
}



