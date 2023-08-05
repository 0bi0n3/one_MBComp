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

    // Set filter parameters
    void setFilterParameters(double cutOffFrequency, double qualityFactor, FilterType filterType);
    
    // Process input sample through filter
    double processFilter(double inputSample, int channelNumber);
    
    // Update method to handle changes in sample rate
    void updateSampleRate(double newSampleRate);
};

// =====================LinkwitzRiley========================

class LinkwitzRFilter
{
    FilterType filterType;
    
    // Low pass and high pass Butterworth filters
    ButterFilter lowPassFilter, highPassFilter, allPassFilter;
    
public:
    // Constructor
    LinkwitzRFilter(double sampleRate);

    // Set crossover frequency
    void setCrossoverFrequency(double crossoverFrequency);
    
    // Process input sample through filters
    double processFilter(double inputSample, int channelNumber);
        
    // Method to set the filter type
    void setType(FilterType newType);
};



#endif /* butterworthFilter_h */
