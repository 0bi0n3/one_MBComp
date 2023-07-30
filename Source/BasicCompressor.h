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
    float previousEnvelopeLevel = 0.0f;
    float threshold_ = -10.0f;
    float ratio_ = 20.0f;
    float tauAttack_ = 2000.0f;
    float tauRelease_ = 6000.0f;
    float makeUpGain_ = 0.0f;

    float alphaAttack;
    float alphaRelease;
    
    juce::dsp::ProcessSpec spec_;

public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::ProcessContextReplacing<float>& context);
    void setAttackTime(float newAttackTime);
    void setReleaseTime(float newReleaseTime);
    void setThresholdLevel(float newThreshold);
    void setRatio(float newRatio);
    void setMakeUpGain(float newMakeUpGain);

};


#endif /* BasicCompressor_h */
