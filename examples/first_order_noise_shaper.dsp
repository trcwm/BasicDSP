%
% First order sigma-delta noise shaper
% Niels A. Moseley, Moseley Instruments
%

% create low-frequency input sinusoid

z = mod1(z + slider1/256);
sig_in = sin1(z);

% calculate loop filter
state = state - ns_out + sig_in*0.75

% calculate noise shaper output
ns_out = sign(state)

out=ns_out

