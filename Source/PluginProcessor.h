/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.
    Oberon Day-West (2023). #21501990.
    This code has been referenced and adapted from Schiermeyer (2021a; 2021b), Pirkle (2019) and Tarr (2019).
    Please refer to the accompanying report for full list of references.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BasicCompressor.h"

namespace PluginParameters
{
enum ParamNames
{
    Low_Mid_XO_Frequency,
    Mid_High_XO_Frequency,
    
    Threshold_LB,
    Threshold_MB,
    Threshold_HB,
    
    Attack_LB,
    Attack_MB,
    Attack_HB,
    
    Release_LB,
    Release_MB,
    Release_HB,
    
    Ratio_LB,
    Ratio_MB,
    Ratio_HB,
    
    Bypass_LB,
    Bypass_MB,
    Bypass_HB,
};

inline const std::map<ParamNames, juce::String>& GetParameters()
{
    static std::map<ParamNames, juce::String> parameters =
    {
        { Low_Mid_XO_Frequency, "Low-Mid Crossover Frequency" },
        { Mid_High_XO_Frequency, "Mid-High Crossover Frequency" },
        
        { Threshold_LB, "Low-Band Threshold" },
        { Threshold_MB, "Mid-Band Threshold" },
        { Threshold_HB, "High-Band Threshold" },
        
        { Attack_LB, "Low-Band Attack" },
        { Attack_MB, "Mid-Band Attack" },
        { Attack_HB, "High-Band Attack" },
    
        { Release_LB, "Low-Band Release" },
        { Release_MB, "Mid-Band Release" },
        { Release_HB, "High-Band Release" },
    
        { Ratio_LB, "Low-Band Ratio" },
        { Ratio_MB, "Mid-Band Ratio" },
        { Ratio_HB, "High-Band Ratio" },
    
        { Bypass_LB, "Low-Band Bypass" },
        { Bypass_MB, "Mid-Band Bypass" },
        { Bypass_HB, "High-Band Bypass" },
    };
    
    return parameters;
}
}

struct CompressorBand
{
private:
    BasicCompressor compressor;
    
public:
    juce::AudioParameterFloat* attackTime = nullptr;
    juce::AudioParameterFloat* releaseTime = nullptr;
    juce::AudioParameterFloat* thresholdLevel = nullptr;
    juce::AudioParameterChoice* ratio = nullptr;
    juce::AudioParameterBool* bypassed = nullptr;
    
    void prepareComp( const juce::dsp::ProcessSpec& spec )
    {
        compressor.prepare(spec);
    }
    
    void updateCompressorParamSettings()
    {
        compressor.setAttackTime( attackTime->get() );
        compressor.setReleaseTime( releaseTime->get() );
        compressor.setThresholdLevel( thresholdLevel->get() );
        compressor.setCompressionRatio( ratio->getCurrentChoiceName().getFloatValue() );
    }
    
    void process(juce::AudioBuffer<float>& buffer)
    {
        auto sampleBlock = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(sampleBlock);
        
        context.isBypassed = bypassed->get();
            
        compressor.process(context);
    }
};

//==============================================================================
/**
*/
class One_MBCompAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    One_MBCompAudioProcessor();
    ~One_MBCompAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // implemented short-hand object access
    using APVTS = juce::AudioProcessorValueTreeState;
    static APVTS::ParameterLayout createParameterLayout();
    
    APVTS apvts { *this, nullptr, "Parameters", createParameterLayout() };

private:
    CompressorBand compressor;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (One_MBCompAudioProcessor)
};
