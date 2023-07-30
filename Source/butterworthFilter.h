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

class Butterworth
{
    // Coefficients for Butterworth filter
    double coefficientA0, coefficientA1, coefficientA2, coefficientB1, coefficientB2;
    
    // Filter parameters
    double cutOffFrequency, qualityFactor;
    
    // Vectors to store previous samples
    std::vector<double> previousSamples1, previousSamples2;

public:
    // Constructor initializes previous samples to 0
    Butterworth() : previousSamples1(2, 0), previousSamples2(2, 0) {}

    // Set filter parameters
    void setFilterParameters(double cutOffFrequency, double qualityFactor);
    
    // Process input sample through filter
    double processFilter(double inputSample, int channelNumber);
};

// =====================LinkwitzRiley========================

class LinkwitzRiley
{
    // Low pass and high pass Butterworth filters
    Butterworth lowPassFilter, highPassFilter;

public:
    // Default constructor
    LinkwitzRiley() {}

    // Set crossover frequency
    void setCrossoverFrequency(double crossoverFrequency);
    
    // Process input sample through low pass filter
    double processLowPassFilter(double inputSample, int channelNumber);
    
    // Process input sample through high pass filter
    double processHighPassFilter(double inputSample, int channelNumber);
};



#endif /* butterworthFilter_hpp */
