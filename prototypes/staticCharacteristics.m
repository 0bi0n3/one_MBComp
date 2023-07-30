%% staticCharacteristics.m
% This script the static characteristics in the detection path of a
% compressor. The static characteristics include the threshold, ratio and
% knee.
% This script has been inspired and adpated from Tarr, E. (2019).

x = [0:.001:1].'; % Test signal, from 0 to 1, in steps of 0.001
N = length(x);

% Static characteristics
Threshold = -50;
Ratio = 4;

for n = 1:N
    x_dB(n,1) = 20 * log10(abs(x(n,1)));
    
    if x_dB(n,1) < -144
        x_dB(n,1) = -144; % Safe guard against negative infinity
    end

    % Compare threshold
    if x_dB(n,1) > Threshold
        % Compress signal
        g_sc(n,1) = Threshold + ((x_dB(n,1) - Threshold) / Ratio);
    else
        % No compression
        g_sc(n,1) = x_dB(n,1);
    end
end

% Plot the above characteristics
plot(x_dB, g_sc);
xlabel('Input Amplitude (dBFS)');
ylabel('Output Amplitude (dBFS)');