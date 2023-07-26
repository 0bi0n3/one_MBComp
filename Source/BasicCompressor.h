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

public:
    void prepare(double sampleRate);
    void process(juce::AudioBuffer<float>& buffer);
};


#endif /* BasicCompressor_h */
