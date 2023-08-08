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
#include "butterworthFilter.h"

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
                       ),
LP1(getSampleRate()), AP2(getSampleRate()), HP1(getSampleRate()), LP2(getSampleRate()), HP2(getSampleRate())
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
    
    floatHelper(inputGainParameter, ParamNames::Gain_Input);
    floatHelper(outputGainParameter, ParamNames::Gain_Output);
    
    LP1.setType(FilterType::lowpass);
    HP1.setType(FilterType::highpass);

    AP2.setType(FilterType::allpass);

    LP2.setType(FilterType::lowpass);
    HP2.setType(FilterType::highpass);
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
    
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    
    inputGain.setRampDurationSeconds(0.05); // ms
    outputGain.setRampDurationSeconds(0.05);
        
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
    // juce::ScopedNoDenormals disables denormalised numbers, which can be a source of
    // performance issues in audio processing.
    juce::ScopedNoDenormals noDenormals;
    
    // Get the total number of input and output channels
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // This loop clears any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Loop through all compressors and update their parameter settings
    for( auto& compressor : compressors )
    {
        compressor.updateCompressorParamSettings();
    }
    
    // Set the input and output gain values in decibels
    inputGain.setGainDecibels( inputGainParameter->get() );
    outputGain.setGainDecibels( outputGainParameter->get() );
    
    // Apply the gain to the buffer
    applyGain(buffer, inputGain);
    
    // Assign the contents of buffer to all elements in the filterBuffers vector
    for( auto& filter_buffer : filterBuffers )
    {
        filter_buffer = buffer;
    }
    
    // Get the crossover frequencies for the filters
    auto filter_lowMidCutoff = lowMidFreqXover->get();
    auto filter_midHighCutoff = midHighFreqXover->get();
    
    // Set the cutoff frequencies for the filters
    LP1.setCrossoverFrequency(filter_lowMidCutoff);
    HP1.setCrossoverFrequency(filter_lowMidCutoff);
    AP2.setCrossoverFrequency(filter_midHighCutoff);
    LP2.setCrossoverFrequency(filter_midHighCutoff);
    HP2.setCrossoverFrequency(filter_midHighCutoff);

    // Create AudioBlocks from the filterBuffers
    auto filter_bufferBlock0 = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto filter_bufferBlock1 = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto filter_bufferBlock2 = juce::dsp::AudioBlock<float>(filterBuffers[2]);
    
    // Create ProcessContexts from the AudioBlocks
    auto filter_bufferContext0 = juce::dsp::ProcessContextReplacing<float>(filter_bufferBlock0);
    auto filter_bufferContext1 = juce::dsp::ProcessContextReplacing<float>(filter_bufferBlock1);
    auto filter_bufferContext2 = juce::dsp::ProcessContextReplacing<float>(filter_bufferBlock2);
    
    // Process the filters in a certain order (flow)
    LP1.process(filter_bufferContext0);
    AP2.process(filter_bufferContext0);
    HP1.process(filter_bufferContext1);
    filterBuffers[2] = filterBuffers[1];
    LP2.process(filter_bufferContext1);
    HP2.process(filter_bufferContext2);
    
    // Loop through all compressors and apply them to the corresponding filter buffer
    for( size_t i = 0; i < filterBuffers.size(); ++i )
    {
        compressors[i].process(filterBuffers[i]);
    }
    
//    auto numberSamples = buffer.getNumSamples();
//    auto numberChannels = buffer.getNumChannels();
    
    // Clear the buffer
    buffer.clear();
    
    // A function that adds a source buffer to the input buffer
    auto addFilterBand = [numberChannels = buffer.getNumChannels(), numberSamples = buffer.getNumSamples()]( auto& inputBuffer, const auto& source )
    {
        for( auto i = 0; i < numberChannels; ++i )
        {
            inputBuffer.addFrom(i, 0, source, i, 0, numberSamples);
        }
    };
    
    // Check if any compressors are soloed
    auto bandsAreSoloed = false;
    for( auto& comp : compressors )
    {
        if( comp.solo->get() )
        {
            bandsAreSoloed = true;
            break;
        }
    }
    
    // If any compressors are soloed, add only those filter bands to the buffer
    // Otherwise, add all unmuted filter bands to the buffer
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
    
    // Apply the output gain to the buffer
    applyGain(buffer, outputGain);
}

//==============================================================================
bool One_MBCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* One_MBCompAudioProcessor::createEditor()
{
    return new One_MBCompAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor(*this);
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
    // Define the layout for the plugin's parameters.
    APVTS::ParameterLayout PluginGUIlayout;
    
    // Import namespaces for JUCE and the parameters used in this plugin.
    using namespace juce;
    using namespace PluginParameters;
    const auto& parameters = GetParameters(); // Get the parameters defined for the plugin.
    
    // ===== Gain parameters
    // Define the range for the Gain parameters, -24 to 24 with steps of 0.5.
    auto gainRangeValues = NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f);
    
    // Add Gain Input and Output parameters to the layout.
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Gain_Input),
                                                              parameters.at(ParamNames::Gain_Input),
                                                              gainRangeValues,
                                                              0));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Gain_Output),
                                                              parameters.at(ParamNames::Gain_Output),
                                                              gainRangeValues,
                                                              0));
    
    // ===== Threshold parameters
    // Add threshold parameters to the layout.
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
   
    // Define the range for the Attack and Release parameters, 5 to 500 with steps of 1.
    auto attkRelRange = NormalisableRange<float>(5, 500, 1, 1);
    
    // ===== Attack parameters
    // Add attack parameters to the layout.
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
    // Add release parameters to the layout.
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
    
    // Define the choices for the Ratio parameter as a vector of doubles.
    auto ratioChoices = std::vector<double>{ 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 8, 10, 15, 20, 50 };
    
    // Convert the ratio choices to a StringArray so they can be used with an AudioParameterChoice.
    juce::StringArray strArr;
    for( auto rChoice : ratioChoices )
    {
        strArr.add( juce::String(rChoice, 1) );
    }
    
    // ===== Ratio parameters
    // Add ratio parameters to the layout.
    PluginGUIlayout.add(std::make_unique<AudioParameterChoice>(parameters.at(ParamNames::Ratio_LB),
                                                               parameters.at(ParamNames::Ratio_LB), strArr, 3));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterChoice>(parameters.at(ParamNames::Ratio_MB),
                                                               parameters.at(ParamNames::Ratio_MB), strArr, 3));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterChoice>(parameters.at(ParamNames::Ratio_HB),
                                                               parameters.at(ParamNames::Ratio_HB), strArr, 3));
    // ===== Bypass parameters
    // Add bypass parameters to the layout.
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Bypass_LB),
                                                             parameters.at(ParamNames::Bypass_LB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Bypass_MB),
                                                             parameters.at(ParamNames::Bypass_MB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Bypass_HB),
                                                             parameters.at(ParamNames::Bypass_HB), false));
    
    // ===== Mute parameters
    // Add mute parameters to the layout.
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Mute_LB),
                                                             parameters.at(ParamNames::Mute_LB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Mute_MB),
                                                             parameters.at(ParamNames::Mute_MB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Mute_HB),
                                                             parameters.at(ParamNames::Mute_HB), false));
    
    // ===== Solo parameters
    // Add solo parameters to the layout.
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Solo_LB),
                                                             parameters.at(ParamNames::Solo_LB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Solo_MB),
                                                             parameters.at(ParamNames::Solo_MB), false));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Solo_HB),
                                                             parameters.at(ParamNames::Solo_HB), false));
    
    // Add Frequency parameters for Low-Mid Crossover and Mid-High Crossover to the layout.
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Low_Mid_XO_Frequency),
                                                             parameters.at(ParamNames::Low_Mid_XO_Frequency),
                                                              NormalisableRange<float>(20, 999, 1, 1), 400));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Mid_High_XO_Frequency),
                                                             parameters.at(ParamNames::Mid_High_XO_Frequency),
                                                              NormalisableRange<float>(1000, 20000, 1, 1), 2000));
    // Return the completed layout.
    return PluginGUIlayout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new One_MBCompAudioProcessor();
}
