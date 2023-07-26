//
//  BasicCompressor.h
//  one_MBComp
//
//  Created by Oberon Day-West on 26/07/2023.
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//

#ifndef BasicCompressor_h
#define BasicCompressor_h

#include <JuceHeader.h>

class BasicCompressor
{
private:
    float yL_prev = 0.0f;
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
    void setThreshold(float newThreshold);
    void setRatio(float newRatio);
    void setMakeUpGain(float newMakeUpGain);

};


#endif /* BasicCompressor_h */
