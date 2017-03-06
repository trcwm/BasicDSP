%
% Second-order sigma-delta noise shaper
% based on the error-feedback topology
%
% Niels A. Moseley, Moseley Instruments
%

% create low-frequency input sinusoid
z = mod1(z + slider1/256);
sig_in = sin1(z)*0.5;

% calculate noise shaper output
ns_out = sign(sig_in - error_fb)

% calculate error
error = sig_in - error_fb - ns_out

% error filter
error_fb = -2.0*error + last_error
last_error = error

% output lowpass/reconstruction filter
f = 0.05
hp = ns_out - lp - bp
bp = bp + f*hp
lp = lp + f*bp

out = lp
