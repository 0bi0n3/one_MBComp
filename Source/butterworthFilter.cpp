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
    

void Butterworth::setFilterParameters(double cutOffFrequency, double qualityFactor)
{
    // Validate filter parameters
    if (cutOffFrequency <= 0 || cutOffFrequency >= 1 || qualityFactor <= 0)
    {
        throw std::invalid_argument("Invalid filter parameters.");
    }

    this->cutOffFrequency = cutOffFrequency;
    this->qualityFactor = qualityFactor;

    // Calculate intermediate variables for Butterworth filter
    double intermediateVariableK = std::tan(M_PI * cutOffFrequency);
    double normalizationFactor = 1 / (1 + intermediateVariableK / qualityFactor + intermediateVariableK * intermediateVariableK);
    
    // Calculate coefficients for Butterworth filter
    coefficientA0 = intermediateVariableK * intermediateVariableK * normalizationFactor;
    coefficientA1 = 2 * coefficientA0;
    coefficientA2 = coefficientA0;
    coefficientB1 = 2 * (intermediateVariableK * intermediateVariableK - 1) * normalizationFactor;
    coefficientB2 = (1 - intermediateVariableK / qualityFactor + intermediateVariableK * intermediateVariableK) * normalizationFactor;
}

double Butterworth::processFilter(double inputSample, int channelNumber)
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

// ======================================================================

void LinkwitzRiley::setCrossoverFrequency(double crossoverFrequency)
{
    // Validate crossover frequency
    if (crossoverFrequency <= 0 || crossoverFrequency >= 1) {
        throw std::invalid_argument("Invalid crossover frequency.");
    }

    // Set crossover frequency for low pass and high pass filter
    lowPassFilter.setFilterParameters(crossoverFrequency, 0.5);
    highPassFilter.setFilterParameters(crossoverFrequency, 0.5);
}

double LinkwitzRiley::processLowPassFilter(double inputSample, int channelNumber)
{
    // Process the input sample with the low pass filter twice
    return lowPassFilter.processFilter(lowPassFilter.processFilter(inputSample, channelNumber), channelNumber);
}

double LinkwitzRiley::processHighPassFilter(double inputSample, int channelNumber)
{
    // Process the input sample with the high pass filter twice
    return highPassFilter.processFilter(highPassFilter.processFilter(inputSample, channelNumber), channelNumber);
}


