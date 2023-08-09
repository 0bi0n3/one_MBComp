/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    This code has been referenced and adapted from Schiermeyer (2021a; 2021b).
 
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

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;
    
    void drawToggleButton (juce::Graphics &g,
                           juce::ToggleButton & toggleButton,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;
};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix, const juce::String& title /*= no title */ ) : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
    {
        setName(title);
        setLookAndFeel(&lnf);
    }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
    
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct PowerButton : juce::ToggleButton {};

//==============================================================================

struct Placeholder : juce::Component
{
    Placeholder();
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(customColour);
    }
    
    juce::Colour customColour;
};

struct RotarySlider : juce::Slider
{
    RotarySlider() :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox)
    {}
};

template<
    typename Attachment,
    typename APVTS,
    typename PluginParameters,
    typename ParamNames,
    typename SliderType>
void makeAttachment(std::unique_ptr<Attachment>& attachment,
                    APVTS& apvts,
                    const PluginParameters& parameters,
                    const ParamNames& name,
                    SliderType& slider)
{
    attachment = std::make_unique<Attachment>(apvts,
                                              parameters.at(name),
                                              slider);
}

template<
    typename APVTS,
    typename PluginParameters,
    typename ParamNames>
juce::RangedAudioParameter& getParameter(APVTS& apvts, const PluginParameters& params, const ParamNames& name)
{
    auto param = apvts.getParameter(params.at(name));
    jassert( param != nullptr );
    
    return *param;
}

juce::String getValString(const juce::RangedAudioParameter& param,
                          bool getLow,
                          juce::String suffix);

template<
    typename Labels,
    typename ParamType,
    typename SuffixType>
void addLabelPairs(Labels& labels, const ParamType& param, const SuffixType& suffix)
{
    labels.clear();
    labels.add({0.f, getValString(param, true, suffix)});
    labels.add({1.f, getValString(param, false, suffix)});
}

struct GlobalControls : juce::Component
{
    GlobalControls(juce::AudioProcessorValueTreeState& apvts);
    
    void paint(juce::Graphics& g) override;
    
    void resized() override;
    
private:
    using RotarySliderWL = RotarySliderWithLabels;
    std::unique_ptr<RotarySliderWL> inputGainSlider, lowMidCrossoverSlider, midHighCrossoverSlider, outputGainSlider;
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> lowMidCrossoverSliderAttachment,
                                midHighCrossoverSliderAttachment,
                                inputGainSliderAttachment,
                                outputGainSliderAttachment;
    
};

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
    
    Placeholder controlBar, analyser, /*globalControls*/ bandControls;
    GlobalControls globalControls { audioProcessor.apvts };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (One_MBCompAudioProcessorEditor)
};
