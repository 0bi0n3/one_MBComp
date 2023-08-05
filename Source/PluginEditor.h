/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

/*
 GUI design steps:
 1) Global controls, gain and crossover.
 2) Mid/main and controls(attack, release, threshold, ratio).
 3) Solo, mute and bypass buttons.
 4) Band selection.
 5) Band selection = same as solo, mute and bypass button states.
 6) Customise buttons and sliders.
 7) Spectrum analyser.
 8) Meter overlay on bands.
 9) meter response to DSP.
 10) Crossover sections on analyser.
 11) Showing gain reduction on bands.
 12) Global bypass button.
 */

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class One_MBCompAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    One_MBCompAudioProcessorEditor (One_MBCompAudioProcessor&);
    ~One_MBCompAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    One_MBCompAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (One_MBCompAudioProcessorEditor)
};
