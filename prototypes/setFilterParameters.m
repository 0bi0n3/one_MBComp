%% Filter parameters
%
% This code has been referenced and adapted from Bristow-Johnson (2005), neotec (2007), Falco (2009) and Zolzer (2011).
% Please refer to accompanying report for full reference list and details.
% Oberon Day-West (21501990).

function coefficients = setFilterParameters(cutOffFrequency, qualityFactor, filterType, sampleRate)
    w0 = 2 * pi * cutOffFrequency / sampleRate;
    alpha = sin(w0) / (2 * qualityFactor);

    if strcmp(filterType, 'lowpass')
        a0 = 1 + alpha;
        coefficientA0 = (1 - cos(w0)) / 2 / a0;
        coefficientA1 = (1 - cos(w0)) / a0;
        coefficientA2 = coefficientA0;
        coefficientB1 = -2 * cos(w0) / a0;
        coefficientB2 = (1 - alpha) / a0;
    elseif strcmp(filterType, 'highpass')
        a0 = 1 + alpha;
        coefficientA0 = (1 + cos(w0)) / 2 / a0;
        coefficientA1 = -(1 + cos(w0)) / a0;
        coefficientA2 = coefficientA0;
        coefficientB1 = -2 * cos(w0) / a0;
        coefficientB2 = (1 - alpha) / a0;
    elseif strcmp(filterType, 'allpass')
        a0 = 1 + alpha;
        coefficientA0 = (1 - alpha) / a0;
        coefficientA1 = -2 * cos(w0) / a0;
        coefficientA2 = (1 + alpha) / a0;
        coefficientB1 = coefficientA1; % same as a1
        coefficientB2 = coefficientA0; % same as a0
    else
        error('Invalid filter type.');
    end

    coefficients = struct('A0', coefficientA0, 'A1', coefficientA1, 'A2', coefficientA2, ...
                          'B1', coefficientB1, 'B2', coefficientB2);
end
