//
//  butterworthFilter.hpp
//  one_MBComp
//
//  
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//  This code has been referenced and adapted from Bristow-Johnson (2005), neotec (2007), Falco (2009) and Zolzer (2011).
//

#ifndef butterworthFilter_hpp
#define butterworthFilter_hpp

#include <cmath>
#include <vector>
#include <stdexcept>

// =====================Butterworth========================

class ButterFilter
{
    // Coefficients for Butterworth filter
    double coefficientA0, coefficientA1, coefficientA2, coefficientB1, coefficientB2;
    
    // Filter parameters
    double cutOffFrequency, qualityFactor;
    
    double sampleRate;
    
    // Vectors to store previous samples
    std::vector<double> previousSamples1, previousSamples2;

public:
    // Constructor
    ButterFilter(double sampleRate);

    // Set filter parameters
    void setFilterParameters(double cutOffFrequency, double qualityFactor);
    
    // Process input sample through filter
    double processFilter(double inputSample, int channelNumber);
    
    // Update method to handle changes in sample rate
    void updateSampleRate(double newSampleRate);
};

// =====================LinkwitzRiley========================

class LinkwitzRFilter
{
    // Low pass and high pass Butterworth filters
    ButterFilter lowPassFilter, highPassFilter;
    
    // Filter type enumeration
    enum Type
    {
        lowpass,
        highpass,
        allpass
    };

    Type filterType;

public:
    // Constructor
    LinkwitzRFilter(double sampleRate);

    // Set crossover frequency
    void setCrossoverFrequency(double crossoverFrequency);
    
    // Process input sample through low pass filter
    double processLowPassFilter(double inputSample, int channelNumber);
    
    // Process input sample through high pass filter
    double processHighPassFilter(double inputSample, int channelNumber);
    
    // Method to set the filter type
    void setType(Type newType);
};



#endif /* butterworthFilter_hpp */
