//
//  BasicCompressor.h
//  one_MBComp
//
//  UWL #21501990.
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//  This code has been referenced and adapted from Reiss and McPherson (2015), Pirkle (2019) and Tarr (2019).
//  Please refer to the accompanying report for full list of references.

#ifndef BasicCompressor_h
#define BasicCompressor_h

#include <JuceHeader.h>

class BasicCompressor
{
private:
    float m_previousEnvelopeLevel = 0.0f;
    float m_thresholdLevelDb = -10.0f;
    float m_compressionRatio = 20.0f;
    float m_attackTimeInMs = 2000.0f;
    float m_releaseTimeInMs = 6000.0f;
    float m_newMakeUpGainDb = 0.0f;

    float m_alphaAttack;
    float m_alphaRelease;
    
    juce::dsp::ProcessSpec m_compressorSpecifications;

public:
    void prepare(const juce::dsp::ProcessSpec& compressorSpec);
    void process(juce::dsp::ProcessContextReplacing<float>& context);
    void setAttackTime(float newAttackTime);
    void setReleaseTime(float newReleaseTime);
    void setThresholdLevel(float newThreshold);
    void setCompressionRatio(float newCompressionRatio);
    void setMakeUpGain(float newMakeUpGain);

};


#endif /* BasicCompressor_h */
