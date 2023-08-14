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
f1 = 500; % Frequency
f2 = 1000; % Frequency
x = sin(2*pi*f1*t) + sin(2*pi*f2*t); % Sum of two sine waves

sampleRate = Fs;
filterType = 'allpass';
previousSamples1 = zeros(2, 1);
previousSamples2 = zeros(2, 1);

spec.sampleRate = Fs;
spec.numChannels = 1;

% Reset samples
previousSamples1 = zeros(spec.numChannels, 1);
previousSamples2 = zeros(spec.numChannels, 1);

% Set filter parameters (cutOffFrequency, qualityFactor, filterType, sampleRate)
coefficients = setFilterParameters(750, 0.707, 'allpass', sampleRate);

% Filter the signal
y = zeros(size(x));
for i = 1:length(x)
    [y(i), previousSamples1, previousSamples2] = processFilter(x(i), 1, previousSamples1, previousSamples2, coefficients);
end

% Plot original and filtered signal
figure;

% Zoom into a smaller window
zoomWindow = 1:round(0.02*Fs); % First 20ms

subplot(4, 1, 1);
plot(t(zoomWindow), x(zoomWindow), 'b-', t(zoomWindow), y(zoomWindow), 'r--');
xlabel('Time (seconds)');
ylabel('Amplitude');
title('Original and Filtered Signal (Zoomed)');
legend('Original', 'Filtered');

subplot(4, 1, 2);
plot(t, x, 'b-', t, y, 'r--');
xlabel('Time (seconds)');
ylabel('Amplitude');
title('Original and Filtered Signal (Full)');
legend('Original', 'Filtered');

% Frequency response of the filter
[H, w] = freqz([coefficients.A0, coefficients.A1, coefficients.A2], [1, coefficients.B1, coefficients.B2]);

% Plotting Magnitude Response with Vertical Line for Cutoff
subplot(4, 1, 3);
plot(w/pi*(Fs/2), 20*log10(abs(H)));
hold on;
line([750 750], [-40 5], 'Color', 'r', 'LineStyle', '--'); % Vertical line for cutoff
hold off;
xlabel('Frequency (Hz)');
ylabel('Gain (dB)');
title('Magnitude Response of Filter');
grid on;

% Plotting Phase Response with Vertical Line for Cutoff
subplot(4, 1, 4);
plot(w/pi*(Fs/2), angle(H));
hold on;
line([750 750], [-pi pi], 'Color', 'r', 'LineStyle', '--'); % Vertical line for cutoff
hold off;
xlabel('Frequency (Hz)');
ylabel('Phase (radians)');
title('Phase Response of Filter');
grid on;

% Magnitude spectra comparison
figure;
f = (0:length(x)-1) * Fs / length(x);
X = abs(fft(x));
Y = abs(fft(y));
subplot(2, 1, 1);
plot(f, 20*log10(X));
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Magnitude Spectrum of Original Signal');
xlim([0 Fs/2]);
grid on;
hold on;
line([750 750], [min(20*log10(X)) max(20*log10(X))], 'Color', 'r', 'LineStyle', '--'); % Vertical line for cutoff
hold off;

subplot(2, 1, 2);
plot(f, 20*log10(Y));
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Magnitude Spectrum of Filtered Signal');
xlim([0 Fs/2]);
grid on;
hold on;
line([750 750], [min(20*log10(Y)) max(20*log10(Y))], 'Color', 'r', 'LineStyle', '--'); % Vertical line for cutoff
hold off;