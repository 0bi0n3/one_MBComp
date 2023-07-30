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
    
    floatHelper(compressor.attackTime, ParamNames::Attack_LB);
    floatHelper(compressor.releaseTime, ParamNames::Release_LB);
    floatHelper(compressor.thresholdLevel, ParamNames::Threshold_LB);

    auto choiceHelper = [&apvts = this->apvts, &parameters](auto& parameter, const auto& ParamNames)
    {
        parameter = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameters.at(ParamNames)));
        jassert( parameter != nullptr );
    };
    
    choiceHelper(compressor.ratio, ParamNames::Ratio_LB);
    
    auto boolHelper = [&apvts = this->apvts, &parameters](auto& parameter, const auto& ParamNames)
    {
        parameter = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(parameters.at(ParamNames)));
        jassert( parameter != nullptr );
    };
    
    boolHelper(compressor.bypassed, ParamNames::Bypass_LB);
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
    
    compressor.prepareComp(spec);
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
    
//    compressor.setAttack( attack->get() );
//    compressor.setRelease( release->get() );
//    compressor.setThreshold( threshold->get() );
//    compressor.setRatio( ratio->getCurrentChoiceName().getFloatValue() );
//
//    auto block = juce::dsp::AudioBlock<float>(buffer);
//    auto context = juce::dsp::ProcessContextReplacing<float>(block);
//
//    context.isBypassed = bypassed->get();
//
//    compressor.process(context);
    
    compressor.updateCompressorParamSettings();
    compressor.process(buffer);
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
    
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Threshold_LB),
                                                              parameters.at(ParamNames::Threshold_LB),
                                                     NormalisableRange<float>(-60, 12, 1, 1),
                                                     0));
    
    auto attkRelRange = NormalisableRange<float>(5, 500, 1, 1);
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Attack_LB),
                                                              parameters.at(ParamNames::Attack_LB),
                                                     attkRelRange,
                                                     50));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterFloat>(parameters.at(ParamNames::Release_LB),
                                                              parameters.at(ParamNames::Release_LB),
                                                     attkRelRange,
                                                     250));
    auto ratioChoices = std::vector<double>{ 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 8, 10, 15, 20, 50 };
    
    juce::StringArray strArr;
    for( auto rChoice : ratioChoices )
    {
        strArr.add( juce::String(rChoice, 1) );
    }
    
    PluginGUIlayout.add(std::make_unique<AudioParameterChoice>(parameters.at(ParamNames::Ratio_LB),
                                                               parameters.at(ParamNames::Ratio_LB), strArr, 3));
    
    PluginGUIlayout.add(std::make_unique<AudioParameterBool>(parameters.at(ParamNames::Bypass_LB),
                                                             parameters.at(ParamNames::Bypass_LB), false));
     
    return PluginGUIlayout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new One_MBCompAudioProcessor();
}
