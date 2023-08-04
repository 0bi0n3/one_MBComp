/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.
     Oberon Day-West (2023). #21501990.
     This code has been referenced and adapted from Schiermeyer (2021a; 2021b), Pirkle (2019) and Tarr (2019).
     Please refer to the accompanying report for full list of references.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BasicCompressor.h"

//==============================================================================
One_MBCompAudioProcessor::One_MBCompAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace PluginParameters;
    const auto& parameters = GetParameters();
    
    auto floatHelper = [&apvts = this->apvts, &parameters](auto& parameter, const auto& ParamNames)
    {
        parameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(parameters.at(ParamNames)));
        jassert( parameter != nullptr );
    };
    
    floatHelper(low_BandCompressor.attackTime,      ParamNames::Attack_LB);
    floatHelper(low_BandCompressor.releaseTime,     ParamNames::Release_LB);
    floatHelper(low_BandCompressor.thresholdLevel,  ParamNames::Threshold_LB);
    
    floatHelper(mid_BandCompressor.attackTime,      ParamNames::Attack_MB);
    floatHelper(mid_BandCompressor.releaseTime,     ParamNames::Release_MB);
    floatHelper(mid_BandCompressor.thresholdLevel,  ParamNames::Threshold_MB);
    
    floatHelper(high_BandCompressor.attackTime,     ParamNames::Attack_HB);
    floatHelper(high_BandCompressor.releaseTime,    ParamNames::Release_HB);
    floatHelper(high_BandCompressor.thresholdLevel, ParamNames::Threshold_HB);

    auto choiceHelper = [&apvts = this->apvts, &parameters](auto& parameter, const auto& ParamNames)
    {
        parameter = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameters.at(ParamNames)));
        jassert( parameter != nullptr );
    };
    
    choiceHelper(low_BandCompressor.ratio,  ParamNames::Ratio_LB);
    choiceHelper(mid_BandCompressor.ratio,  ParamNames::Ratio_MB);
    choiceHelper(high_BandCompressor.ratio, ParamNames::Ratio_HB);
   
    
    auto boolHelper = [&apvts = this->apvts, &parameters](auto& parameter, const auto& ParamNames)
    {
        parameter = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(parameters.at(ParamNames)));
        jassert( parameter != nullptr );
    };
    
    boolHelper(low_BandCompressor.bypassed, ParamNames::Bypass_LB);
    boolHelper(mid_BandCompressor.bypassed, ParamNames::Bypass_MB);
    boolHelper(high_BandCompressor.bypassed, ParamNames::Bypass_HB);
    
    boolHelper(low_BandCompressor.mute, ParamNames::Mute_LB);
    boolHelper(mid_BandCompressor.mute, ParamNames::Mute_MB);
    boolHelper(high_BandCompressor.mute, ParamNames::Mute_HB);
    
    boolHelper(low_BandCompressor.solo, ParamNames::Solo_LB);
    boolHelper(mid_BandCompressor.solo, ParamNames::Solo_MB);
    boolHelper(high_BandCompressor.solo, ParamNames::Solo_HB);
    
    floatHelper(lowMidFreqXover, ParamNames::Low_Mid_XO_Frequency);
    floatHelper(midHighFreqXover, ParamNames::Mid_High_XO_Frequency);
    
    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    
    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
//    invAP1.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
//    invAP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

}

One_MBCompAudioProcessor::~One_MBCompAudioProcessor()
{
}

//==============================================================================
const juce::String One_MBCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool One_MBCompAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool One_MBCompAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool One_MBCompAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double One_MBCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int One_MBCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int One_MBCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void One_MBCompAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String One_MBCompAudioProcessor::getProgramName (int index)
{
    return {};
}

void One_MBCompAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void One_MBCompAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    
    for( auto& comp : compressors )
    {
        comp.prepareComp(spec);
    }
    
    LP1.prepare(spec);
    HP1.prepare(spec);
    AP2.prepare(spec);
    LP2.prepare(spec);
    HP2.prepare(spec);
    
//    invAP1.prepare(spec);
//    invAP2.prepare(spec);
//    invAPBuffer.setSize(spec.numChannels, samplesPerBlock);
        
    for( auto& buffer : filterBuffers )
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }
}

void One_MBCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool One_MBCompAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void One_MBCompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for( auto& compressor : compressors )
    {
        compressor.updateCompressorParamSettings();
        //    compressor.process(buffer);
    }
    
    for( auto& filter_buffer : filterBuffers )
    {
        filter_buffer = buffer;
    }
    
//    invAPBuffer = buffer;
    
    auto filter_lowMidCutoff = lowMidFreqXover->get();
    LP1.setCutoffFrequency(filter_lowMidCutoff);
    HP1.setCutoffFrequency(filter_lowMidCutoff);
//    invAP1.setCutoffFrequency(filter_lowMidCutoff);
    
    auto filter_midHighCutoff = midHighFreqXover->get();
    AP2.setCutoffFrequency(filter_midHighCutoff);
    LP2.setCutoffFrequency(filter_midHighCutoff);
    HP2.setCutoffFrequency(filter_midHighCutoff);
//    invAP2.setCutoffFrequency(filter_midHighCutoff);

    auto filter_bufferBlock0 = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto filter_bufferBlock1 = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto filter_bufferBlock2 = juce::dsp::AudioBlock<float>(filterBuffers[2]);
    
    auto filter_bufferContext0 = juce::dsp::ProcessContextReplacing<float>(filter_bufferBlock0);
    auto filter_bufferContext1 = juce::dsp::ProcessContextReplacing<float>(filter_bufferBlock1);
    auto filter_bufferContext2 = juce::dsp::ProcessContextReplacing<float>(filter_bufferBlock2);
    
    /* filter flow
     LP1 -> AP2 = -----\
     HP1 -> LP2 =      /-----\
     HP1 -> HP2 =            /-----
     */
    
    
    LP1.process(filter_bufferContext0);
    AP2.process(filter_bufferContext0);
    
    HP1.process(filter_bufferContext1);
    filterBuffers[2] = filterBuffers[1];
    LP2.process(filter_bufferContext1);
    
    HP2.process(filter_bufferContext2);
    
    for( size_t i = 0; i < filterBuffers.size(); ++i )
    {
        compressors[i].process(filterBuffers[i]);
    }
    
    auto numberSamples = buffer.getNumSamples();
    auto numberChannels = buffer.getNumChannels();
        
    buffer.clear();
    
    auto addFilterBand = [nc = numberChannels, ns = numberSamples]( auto& inputBuffer, const auto& source )
    {
        for( auto i = 0; i < nc; ++i )
        {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        }
    };
    
    auto bandsAreSoloed = false;
    for( auto& comp : compressors )
    {
        if( comp.solo->get() )
        {
            bandsAreSoloed = true;
            break;
        }
    }
        
//    addFilterBand(buffer, filterBuffers[0]);
//    addFilterBand(buffer, filterBuffers[1]);
//    addFilterBand(buffer, filterBuffers[2]);
    
    if( bandsAreSoloed )
    {
        for( size_t i = 0; i < compressors.size(); ++i )
        {
            auto& comp = compressors[i];
            if( comp.solo->get() )
            {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }
    else
    {
        for( size_t i = 0; i < compressors.size(); ++i )
        {
            auto& comp = compressors[i];
            if( ! comp.mute->get() )
            {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }
    
}

//==============================================================================
bool One_MBCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* One_MBCompAudioProcessor::createEditor()
{
//    return new One_MBCompAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void One_MBCompAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream memoryOutputStream(destData, true);
    apvts.state.writeToStream(memoryOutputStream);
}

void One_MBCompAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto savedTree = juce::ValueTree::readFromData(data, sizeInBytes);
    if( savedTree.isValid() )
    {
        apvts.replaceState(savedTree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout One_MBCompAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout PluginGUIlayout;
    
    using namespace juce;
    using namespace PluginParameters;
    const auto& parameters = GetParameters();
    
    // ===== Threshold parameters
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Threshold_LB),
                                                              parameters.at(ParamNames::Threshold_LB),
                                                     NormalisableRange<float>(-60, 12, 1, 1),
                                                     0));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Threshold_MB),
                                                              parameters.at(ParamNames::Threshold_MB),
                                                     NormalisableRange<float>(-60, 12, 1, 1),
                                                     0));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Threshold_HB),
                                                              parameters.at(ParamNames::Threshold_HB),
                                                     NormalisableRange<float>(-60, 12, 1, 1),
                                                     0));
    
    auto attkRelRange = NormalisableRange<float>(5, 500, 1, 1);
    
    // ===== Attack parameters
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Attack_LB),
                                                              parameters.at(ParamNames::Attack_LB),
                                                     attkRelRange,
                                                     50));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Attack_MB),
                                                              parameters.at(ParamNames::Attack_MB),
                                                     attkRelRange,
                                                     50));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Attack_HB),
                                                              parameters.at(ParamNames::Attack_HB),
                                                     attkRelRange,
                                                     50));
    // ===== Release parameters
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Release_LB),
                                                              parameters.at(ParamNames::Release_LB),
                                                     attkRelRange,
                                                     250));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Release_MB),
                                                              parameters.at(ParamNames::Release_MB),
                                                     attkRelRange,
                                                     250));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Release_HB),
                                                              parameters.at(ParamNames::Release_HB),
                                                     attkRelRange,
                                                     250));
    
    auto ratioChoices = std::vector<double>{ 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 8, 10, 15, 20, 50 };
    
    juce::StringArray strArr;
    for( auto rChoice : ratioChoices )
    {
        strArr.add( juce::String(rChoice, 1) );
    }
    
    // ===== Release parameters
    PluginGUIlayout.add(std::make_unique<AudioParameterChoice>(parameters.at(ParamNames::Ratio_LB),
                                                               parameters.at(ParamNames::Ratio_LB), strArr, 3));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterChoice>(parameters.at(ParamNames::Ratio_MB),
                                                               parameters.at(ParamNames::Ratio_MB), strArr, 3));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterChoice>(parameters.at(ParamNames::Ratio_HB),
                                                               parameters.at(ParamNames::Ratio_HB), strArr, 3));
    // ===== Bypass parameters
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Bypass_LB),
                                                             parameters.at(ParamNames::Bypass_LB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Bypass_MB),
                                                             parameters.at(ParamNames::Bypass_MB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Bypass_HB),
                                                             parameters.at(ParamNames::Bypass_HB), false));
    
    // ===== Mute parameters
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Mute_LB),
                                                             parameters.at(ParamNames::Mute_LB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Mute_MB),
                                                             parameters.at(ParamNames::Mute_MB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Mute_HB),
                                                             parameters.at(ParamNames::Mute_HB), false));
    
    // ===== Solo parameters
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Solo_LB),
                                                             parameters.at(ParamNames::Solo_LB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Solo_MB),
                                                             parameters.at(ParamNames::Solo_MB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Solo_HB),
                                                             parameters.at(ParamNames::Solo_HB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Low_Mid_XO_Frequency),
                                                             parameters.at(ParamNames::Low_Mid_XO_Frequency),
                                                              NormalisableRange<float>(20, 999, 1, 1), 400));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Mid_High_XO_Frequency),
                                                             parameters.at(ParamNames::Mid_High_XO_Frequency),
                                                              NormalisableRange<float>(1000, 20000, 1, 1), 2000));
    
    return PluginGUIlayout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new One_MBCompAudioProcessor();
}
