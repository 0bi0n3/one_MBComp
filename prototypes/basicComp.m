%% Basic compressor
%
% This script was adapted and referenced from Reiss and McPherson (2015) and Tarr (2019).
% Please refer to accompanying report for full reference list and details.
% Oberon Day-West (21501990).
%%
clc; clear;

% Prepare input signal
[inputBuffer, Fs] = audioread('audioLoop1.wav');
inputBuffer = mean(inputBuffer, 4);
bufferSize = length(inputBuffer);
samplerate = 44100;   % Sample rate

% Initialize parameters
yL_prev = 0;
x_g = zeros(bufferSize, 1);
x_l = zeros(bufferSize, 1);
y_g = zeros(bufferSize, 1);
y_l = zeros(bufferSize, 1);
c = zeros(bufferSize, 1);

threshold_ = -10;   % Set threshold (db)
ratio_ = 20;       % Set ratio (10 = 10:1)
tauAttack_ = 2000;   % Set attack time constant (ms)
tauRelease_ = 6000;  % Set release time constant (ms)
makeUpGain_ = 0;  % Set makeup gain (dB)

% Compression: calculates the control voltage
alphaAttack = exp(-1/(tauAttack_ * samplerate / 1000));
alphaRelease = exp(-1/(tauRelease_ * samplerate / 1000));

for i = 1:bufferSize
    % Level detection- estimate level using peak detector
    if abs(inputBuffer(i)) < 0.000001
        x_g(i) = -120;
    else
        x_g(i) = 20*log10(abs(inputBuffer(i)));
    end
    
    % Gain computer - static apply input/output curve
    if x_g(i) >= threshold_
        y_g(i) = threshold_ + (x_g(i)-threshold_) / ratio_;
    else
        y_g(i) = x_g(i);
    end
    x_l(i) = x_g(i) - y_g(i);
    
   % Ballistics - smoothing of the gain
    if x_l(i) > yL_prev
        y_l(i) = alphaAttack * yL_prev + (1 - alphaAttack) * x_l(i);
    else
        y_l(i) = alphaRelease * yL_prev + (1 - alphaRelease) * x_l(i);
    end
    
    % Find control
    c(i) = 10^((makeUpGain_ - y_l(i)) / 20);
    yL_prev = y_l(i);
end

% Apply control voltage to the audio signal
outputBuffer = inputBuffer .* c;

% Create time vector for plotting
t = (1:bufferSize) / samplerate;

% Specify zoom start and end in seconds
zoomStart_s = 30;  % start at 30 seconds
zoomEnd_s = 40;    % end at 40 seconds

% Convert to sample indices
zoomStart = round(zoomStart_s * samplerate);  
zoomEnd = round(zoomEnd_s * samplerate);  

% Plot results
figure
subplot(4,1,1);
plot(t, inputBuffer);
title('Input Signal');
xlabel('Time (s)');
ylabel('Amplitude');

subplot(4,1,2);
plot(t, outputBuffer);
title('Output Signal');
xlabel('Time (s)');
ylabel('Amplitude');

subplot(4,1,3);
plot(t(zoomStart:zoomEnd), inputBuffer(zoomStart:zoomEnd));
hold on;
plot(t(zoomStart:zoomEnd), c(zoomStart:zoomEnd), 'r');
title('Zoomed Input and Control Signal');
xlabel('Time (s)');
ylabel('Amplitude');
legend('Input','Control');

subplot(4,1,4);
plot(t(zoomStart:zoomEnd), outputBuffer(zoomStart:zoomEnd));
title('Zoomed Output Signal');
xlabel('Time (s)');
ylabel('Amplitude');