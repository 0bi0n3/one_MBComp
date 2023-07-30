%% Butterworth
%
% This code has been referenced and adapted from Bristow-Johnson (2005), neotec (2007), Falco (2009) and Zolzer (2011).
% Please refer to accompanying report for full reference list and details.
% Oberon Day-West (21501990).
%%
clc; clear;
% Sample rate (in Hz)
Fs = 48000;

% Create an impulse signal
impulse = [1, zeros(1, 99)];

% Define the cut-off frequency and quality factor
cutOffFrequency = 0.3;  % (30% of Nyquist frequency)
qualityFactor = 0.9;    % Q factor

% Calculate intermediate variables for Butterworth filter
intermediateVariableK = tan(pi * cutOffFrequency);
normalizationFactor = 1 / (1 + intermediateVariableK / qualityFactor + intermediateVariableK * intermediateVariableK);

% Calculate coefficients for Butterworth filter
coefficientA0 = intermediateVariableK * intermediateVariableK * normalizationFactor;
coefficientA1 = 2 * coefficientA0;
coefficientA2 = coefficientA0;
coefficientB1 = 2 * (intermediateVariableK * intermediateVariableK - 1) * normalizationFactor;
coefficientB2 = (1 - intermediateVariableK / qualityFactor + intermediateVariableK * intermediateVariableK) * normalizationFactor;

% Create array to store previous samples
previousSamples1 = zeros(1, 2);
previousSamples2 = zeros(1, 2);

% Choose channel (0 or 1 for stereo)
channelNumber = 1;

% Process the impulse signal
for i = 1:length(impulse)
    inputSample = impulse(i);

    % Filter the input sample
    outputSample = coefficientA0 * inputSample + coefficientA1 * previousSamples1(channelNumber) + coefficientA2 * previousSamples2(channelNumber) - coefficientB1 * previousSamples1(channelNumber) - coefficientB2 * previousSamples2(channelNumber);

    % Update previous samples
    previousSamples2(channelNumber) = previousSamples1(channelNumber);
    previousSamples1(channelNumber) = outputSample;

    % Store output sample
    impulse(i) = outputSample;
end

% Plot the impulse response
figure;
stem(impulse);
title('Impulse Response');

% Calculate the FFT of the impulse response
impulseFFT = fft(impulse);

% Plot the magnitude response
figure;
plot(abs(impulseFFT));
title('Magnitude Response');
xlabel('Frequency (Hz)');
ylabel('Magnitude');

% Plot the phase response
figure;
plot(angle(impulseFFT));
title('Phase Response');
xlabel('Frequency (Hz)');
ylabel('Phase (rad)');

