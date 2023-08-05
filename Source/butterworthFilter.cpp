//
//  butterworthFilter.cpp
//  one_MBComp
//
//
//  Copyright © 2023 Oberon Day-West. All rights reserved.
//  This code has been referenced and adapted from Bristow-Johnson (2005), neotec (2007), Falco (2009) and Zolzer (2011).
//

#include <cmath>
#include <vector>
#include <stdexcept>

#include "butterworthFilter.h"

// Constructor definition
ButterFilter::ButterFilter(double sampleRate, FilterType type) :    sampleRate(sampleRate),
                                                                    previousSamples1(2, 0),
                                                                    previousSamples2(2, 0)
{}

void ButterFilter::setFilterParameters(double cutOffFrequency, double qualityFactor, FilterType filterType)
{
    // Validate filter parameters
    if (cutOffFrequency <= 0 || cutOffFrequency >= 1 || qualityFactor <= 0)
    {
        throw std::invalid_argument("Invalid filter parameters.");
    }

    this->cutOffFrequency = cutOffFrequency;
    this->qualityFactor = qualityFactor;
    this->filterType = filterType;

    // Calculate w0 and alpha based on cutOffFrequency and qualityFactor
        double w0 = 2 * M_PI * cutOffFrequency / sampleRate;
        double alpha = std::sin(w0) / (2 * qualityFactor);

        // Calculate coefficients based on filter type
        if (filterType == FilterType::lowpass)
        {
            double a0 = 1 + alpha;
            coefficientA0 = (1 - std::cos(w0)) / 2 / a0;
            coefficientA1 = (1 - std::cos(w0)) / a0;
            coefficientA2 = coefficientA0;
            coefficientB1 = -2 * std::cos(w0) / a0;
            coefficientB2 = (1 - alpha) / a0;
        }
        else if (filterType == FilterType::highpass)
        {
            double a0 = 1 + alpha;
            coefficientA0 = (1 + std::cos(w0)) / 2 / a0;
            coefficientA1 = -(1 + std::cos(w0)) / a0;
            coefficientA2 = coefficientA0;
            coefficientB1 = -2 * std::cos(w0) / a0;
            coefficientB2 = (1 - alpha) / a0;
        }
        else if (filterType == FilterType::allpass)
        {
            double a0 = 1 + alpha;
            coefficientA0 = (1 - alpha) / a0;
            coefficientA1 = -2 * std::cos(w0) / a0;
            coefficientA2 = (1 + alpha) / a0;
            coefficientB1 = coefficientA1; // same as a1
            coefficientB2 = coefficientA0; // same as a0
        }
        else
        {
            throw std::invalid_argument("Invalid filter type.");
        }
}

double ButterFilter::processFilter(double inputSample, int channelNumber)
{
    // Validate channel index
    if (channelNumber < 0 || channelNumber >= previousSamples1.size())
    {
        throw std::out_of_range("Invalid channel index.");
    }

    // Filter the input sample
    double outputSample = coefficientA0 * inputSample + coefficientA1 * previousSamples1[channelNumber] + coefficientA2 * previousSamples2[channelNumber] - coefficientB1 * previousSamples1[channelNumber] - coefficientB2 * previousSamples2[channelNumber];
    
    // Update previous samples
    previousSamples2[channelNumber] = previousSamples1[channelNumber];
    previousSamples1[channelNumber] = outputSample;
    
    // Return filtered sample
    return outputSample;
}

void ButterFilter::updateSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    // Recalculate the filter parameters with the new sample rate
    setFilterParameters(cutOffFrequency, qualityFactor, filterType);
}

// ======================================================================
// Constructor definition
LinkwitzRFilter::LinkwitzRFilter(double sampleRate) :   lowPassFilter(sampleRate, FilterType::lowpass),
                                                        highPassFilter(sampleRate, FilterType::highpass),
                                                        allPassFilter(sampleRate, FilterType::allpass)
{}

void LinkwitzRFilter::setType(FilterType newType)
{
    filterType = newType;
}

void LinkwitzRFilter::setCrossoverFrequency(double crossoverFrequency)
{
    // Validate crossover frequency
    if (crossoverFrequency <= 0 || crossoverFrequency >= 1) {
        throw std::invalid_argument("Invalid crossover frequency.");
    }

    // Set crossover frequency for low pass and high pass filter
    lowPassFilter.setFilterParameters(crossoverFrequency, 0.707, FilterType::lowpass);
    highPassFilter.setFilterParameters(crossoverFrequency, 0.707, FilterType::highpass);
    allPassFilter.setFilterParameters(crossoverFrequency, 0.707, FilterType::allpass);
}

double LinkwitzRFilter::processFilter(double inputSample, int channelNumber)
{
    // Check for filter type and process accordingly
    if (filterType == FilterType::lowpass)
    {
        // Process the input sample with the low pass filter twice
        return lowPassFilter.processFilter(lowPassFilter.processFilter(inputSample, channelNumber), channelNumber);
    }
    else if (filterType == FilterType::highpass)
    {
        // Process the input sample with the high pass filter twice
        return highPassFilter.processFilter(highPassFilter.processFilter(inputSample, channelNumber), channelNumber);
    }
    else // allpass
    {
        return allPassFilter.processFilter(allPassFilter.processFilter(inputSample, channelNumber), channelNumber);
    }
    return 0.0; // Default return value
}


