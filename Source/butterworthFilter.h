//
//  butterworthFilter.hpp
//  one_MBComp
//
//  
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//  This code has been referenced and adapted from Bristow-Johnson (2005), neotec (2007), Falco (2009) and Zolzer (2011).
//

#ifndef butterworthFilter_h
#define butterworthFilter_h
#include <JuceHeader.h>
#include <cmath>
#include <vector>
#include <stdexcept>

enum class FilterType
{
    lowpass,
    highpass,
    allpass
};

// =====================Butterworth========================

class ButterFilter
{
    FilterType filterType;
    
    // Coefficients for Butterworth filter
    double coefficientA0, coefficientA1, coefficientA2, coefficientB1, coefficientB2;
    
    // Filter parameters
    double cutOffFrequency, qualityFactor;
    
    double sampleRate;
    
    // Vectors to store previous samples
    std::vector<double> previousSamples1, previousSamples2;

public:
    // Constructor
    ButterFilter(double sampleRate, FilterType type);
    
    void prepare(const juce::dsp::ProcessSpec& spec);

    // Set filter parameters
    void setFilterParameters(double cutOffFrequency, double qualityFactor, FilterType filterType);
    
    // Process input sample through filter
    double processFilter(double inputSample, int channelNumber);
    
    // Update method to handle changes in sample rate
    void updateSampleRate(double newSampleRate);
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context);
};

// =====================LinkwitzRiley========================

class LinkwitzRFilter
{
    FilterType filterType;
    
public:
    // Low pass and high pass Butterworth filters
    ButterFilter lowPassFilter, highPassFilter, allPassFilter;
    
    // Constructor
    LinkwitzRFilter(double sampleRate);
    
    void prepare(const juce::dsp::ProcessSpec& spec);

    // Set crossover frequency
    void setCrossoverFrequency(double crossoverFrequency);
    
    // Process input sample through filters
    double processFilter(double inputSample, int channelNumber);
        
    // Method to set the filter type
    void setType(FilterType newType);
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context);
};



#endif /* butterworthFilter_h */
