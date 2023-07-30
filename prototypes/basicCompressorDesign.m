%% Basic compressor
% This script has been referenced and adapted from the basicComp.m example
% provided in Hack Audio by Eric Tarr.


% Step input signal
Fs = 48000;
Ts = 1/Fs;
x = [zeros(Fs,1); ones(Fs,1); zeros(Fs,1)];
N = length(x);

% Parameters for compressor
T = -12; % Compressor threshold in dBFS
R = 3; % Ratio for compression

responseTime = 0.25; % Time in seconds
alpha = exp(-log(9)/(Fs * responseTime));
gainSmoothPrev = 0; % Initalise smoothing of variable

y = zeros(N,1);
lin_A = zeros(N,1);
% Loop over each sample to see if it is above threshold
for n = 1:N
    %%%%% Calculations
    % Turn the input signal into a unipolar signal on the dB scale
    x_uni = abs(x(n,1));
    x_dB = 20*log10(x_uni/1);
    % Ensure there are no values of negative infinity
    if x_dB < -96
        x_dB = -96;
    end

    % Static characteristics
    if x_dB > T
        gainSC = T + (x_dB - T)/R; % perform downward compression
    else
        gainSC = x_dB; % bypass
    end

    gainChange_dB = gainSC - x_dB;

    % Smooth over the gainChange_dB to alter response time
    gainSmooth = ((1 - alpha) * gainChange_dB) + (alpha * gainSmoothPrev);

    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainSmooth/20);

    %%%%%% Apply linear amplitude from detection path to input sample
    y(n,1) = lin_A(n,1) * x(n,1);

    % Update gainSmoothPrev used in the next sample of the loop
    gainSmoothPrev = gainSmooth;

end


t = [0:N-1] * Ts; t = t(:);

subplot(3,1,1);
plot(t,x); title('Step Input'); axis([0 3 -0.1 1.1]);
subplot(3,1,2);
plot(t,y); title('Compressor output'); axis([0 3 -0.1 1.1]);
subplot(3,1,3);
plot(t,lin_A); title('Gain Reduction'); axis([0 3 -0.1 1.1]);
