%% Linkwitz/Butterworth
%
% This code has been referenced and adapted from Bristow-Johnson (2005), neotec (2007), Falco (2009) and Zolzer (2011).
% Please refer to accompanying report for full reference list and details.
% Oberon Day-West (21501990).

%%
clc; clear;

% Initialization
Fs = 44100; % Sample rate
t = 0:1/Fs:1; % 1 second of data
f1 = 3000; % Frequency
f2 = 10000; % Frequency
x = sin(2*pi*f1*t) + sin(2*pi*f2*t); % Sum of two sine waves

sampleRate = Fs;
filterType = 'highpass';
previousSamples1 = zeros(2, 1);
previousSamples2 = zeros(2, 1);

spec.sampleRate = Fs;
spec.numChannels = 1;

% Reset samples
previousSamples1 = zeros(spec.numChannels, 1);
previousSamples2 = zeros(spec.numChannels, 1);

% Set filter parameters (cutOffFrequency, qualityFactor, filterType, sampleRate)
coefficients = setFilterParameters(5000, 0.5, 'highpass', sampleRate);

% Filter the signal
y = zeros(size(x));
for i = 1:length(x)
    [y(i), previousSamples1, previousSamples2] = processFilter(x(i), 1, previousSamples1, previousSamples2, coefficients);
end

% Frequency response of the filter
[H, w] = freqz([coefficients.A0, coefficients.A1, coefficients.A2], [1, coefficients.B1, coefficients.B2]);

% Magnitude Response with Logarithmic Frequency Scale
figure;
semilogx(w/pi*(Fs/2), 20*log10(abs(H)));
xlabel('Frequency (Hz)');
ylabel('Gain (dB)');
title('Filter Magnitude Response');
grid on;
xlim([100 Fs/2]);

% Phase Response
figure;
semilogx(w/pi*(Fs/2), unwrap(angle(H))*180/pi);
xlabel('Frequency (Hz)');
ylabel('Phase (degrees)');
title('Filter Phase Response');
grid on;
xlim([100 Fs/2]);

% Magnitude Spectra Comparison with Zoom
figure;
f = (0:length(x)-1) * Fs / length(x);
X = abs(fft(x));
Y = abs(fft(y));
subplot(2, 1, 1);
semilogx(f, 20*log10(X));
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Magnitude Spectrum of Original Signal');
xlim([1000 20000]); % Adjustable
grid on;
subplot(2, 1, 2);
semilogx(f, 20*log10(Y));
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Magnitude Spectrum of Filtered Signal');
xlim([1000 20000]); % Adjustable
grid on;