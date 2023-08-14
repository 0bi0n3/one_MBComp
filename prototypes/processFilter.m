%% Filter processing
%
% This code has been referenced and adapted from Bristow-Johnson (2005), neotec (2007), Falco (2009) and Zolzer (2011).
% Please refer to accompanying report for full reference list and details.
% Oberon Day-West (21501990).

function [outputSample, previousSamples1, previousSamples2] = processFilter(inputSample, channelNumber, previousSamples1, previousSamples2, coefficients)
    outputSample = coefficients.A0 * inputSample + coefficients.A1 * previousSamples1(channelNumber) + coefficients.A2 * previousSamples2(channelNumber) - coefficients.B1 * previousSamples1(channelNumber) - coefficients.B2 * previousSamples2(channelNumber);
    
    % Update previous samples
    previousSamples2(channelNumber) = previousSamples1(channelNumber);
    previousSamples1(channelNumber) = outputSample;
end
