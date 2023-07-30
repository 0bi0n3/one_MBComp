%% dBConversion.m
% This script has been inspired and adpated from Tarr, E. (2019).
% Converting linear amplitude from signal to decibel scale.

x = [-1:.01:1].'; % Linear amp vales over FS
N = length(x);

for n = 1:N
    x_dB(n,1) = 20 * log10(abs(x(n,1))); % converting to dB
    if x_dB(n,1) < -144
        x_dB(n,1) = -144 % prevents negative inf
    end
end

plot(x_dB);