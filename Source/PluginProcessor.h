/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.
    Oberon Day-West (2023). #21501990.
    This code has been referenced and adapted from Schiermeyer (2021a; 2021b), Pirkle (2019) and Tarr (2019).
    Please refer to the accompanying report for full list of references.

  ==============================================================================
*/

#pragma once

#include <array>
#include <JuceHeader.h>
#include "BasicCompressor.h"
#include "butterworthFilter.h"

/*
  ==============================================================================
    **************************************************************************
    NOTE TO READER: This following section of code was explicitly referenced from
    Schiermeyer (2021a; 2021b) to demonstrate the spectrum in the plugin. This
    code block has not be originally created by the author. The end of the block
    will be clearly stated for ease of reference to the reader.
    **************************************************************************
  ==============================================================================
*/

template<typename T>
struct Fifo
{
    void prepare(int numChannels, int numSamples)
    {
        static_assert( std::is_same_v<T, juce::AudioBuffer<float>>,
                      "prepare(numChannels, numSamples) should only be used when the Fifo is holding juce::AudioBuffer<float>");
        for( auto& buffer : buffers)
        {
            buffer.setSize(numChannels,
                           numSamples,
                           false,   //clear everything?
                           true,    //including the extra space?
                           true);   //avoid reallocating if you can?
            buffer.clear();
        }
    }
    
    void prepare(size_t numElements)
    {
        static_assert( std::is_same_v<T, std::vector<float>>,
                      "prepare(numElements) should only be used when the Fifo is holding std::vector<float>");
        for( auto& buffer : buffers )
        {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }
    
    bool push(const T& t)
    {
        auto write = fifo.write(1);
        if( write.blockSize1 > 0 )
        {
            buffers[write.startIndex1] = t;
            return true;
        }
        
        return false;
    }
    
    bool pull(T& t)
    {
        auto read = fifo.read(1);
        if( read.blockSize1 > 0 )
        {
            t = buffers[read.startIndex1];
            return true;
        }
        
        return false;
    }
    
    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }
private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo {Capacity};
};

enum Channel
{
    Right, //effectively 0
    Left //effectively 1
};

template<typename BlockType>
struct SingleChannelSampleFifo
{
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch)
    {
        prepared.set(false);
    }
    
    void update(const BlockType& buffer)
    {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse );
        auto* channelPtr = buffer.getReadPointer(channelToUse);
        
        for( int i = 0; i < buffer.getNumSamples(); ++i )
        {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);
        
        bufferToFill.setSize(1,             //channel
                             bufferSize,    //num samples
                             false,         //keepExistingContent
                             true,          //clear extra space
                             true);         //avoid reallocating
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }
    //==============================================================================
    int getNumCompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading(); }
    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }
    //==============================================================================
    bool getAudioBuffer(BlockType& buf) { return audioBufferFifo.pull(buf); }
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;
    
    void pushNextSampleIntoFifo(float sample)
    {
        if (fifoIndex == bufferToFill.getNumSamples())
        {
            auto ok = audioBufferFifo.push(bufferToFill);

            juce::ignoreUnused(ok);
            
            fifoIndex = 0;
        }
        
        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
};

/*
  ==============================================================================
    **************************************************************************
                                END OF CODE BLOCK
                            (Schiermeyer, 2021a; 2021b)
    **************************************************************************
  ==============================================================================
*/

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
    
    Mute_LB,
    Mute_MB,
    Mute_HB,
    
    Solo_LB,
    Solo_MB,
    Solo_HB,
    
    Gain_Input,
    Gain_Output,
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
        
        { Mute_LB, "Low-Band Mute" },
        { Mute_MB, "Mid-Band Mute" },
        { Mute_HB, "High-Band Mute" },
        
        { Solo_LB, "Low-Band Solo" },
        { Solo_MB, "Mid-Band Solo" },
        { Solo_HB, "High-Band Solo" },
        
        { Gain_Input, "Gain Input" },
        { Gain_Output, "Gain Output" },
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
    juce::AudioParameterBool* mute = nullptr;
    juce::AudioParameterBool* solo = nullptr;
    
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
    
    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo{Channel::Left};
    SingleChannelSampleFifo<BlockType> rightChannelFifo{Channel::Right};

private:
    std::array<CompressorBand, 3> compressors;
    CompressorBand& low_BandCompressor = compressors[0];
    CompressorBand& mid_BandCompressor = compressors[1];
    CompressorBand& high_BandCompressor = compressors[2];
    
//    LinkwitzRiley LPF, HPF;
    using Filters = LinkwitzRFilter;
    //      FC0     FC1
    Filters LP1,    AP2,
            HP1,    LP2,
                    HP2;
    
    juce::AudioParameterFloat* lowMidFreqXover { nullptr };
    juce::AudioParameterFloat* midHighFreqXover { nullptr };
    
    std::array<juce::AudioBuffer<float>, 3> filterBuffers;
    
    juce::dsp::Gain<float> inputGain, outputGain;
    juce::AudioParameterFloat* inputGainParameter { nullptr };
    juce::AudioParameterFloat* outputGainParameter { nullptr };
    
    template<typename T, typename U>
    void applyGain(T& buffer, U& gain)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        gain.process(context);
    }
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (One_MBCompAudioProcessor)
};
