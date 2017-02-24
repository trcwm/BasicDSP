% 
% PLL for BasicDSP 2.0
%
% Assumption: left channel (inL) has a sinusoidal signal
% at around 500 Hz
%

% set the VCO base frequency
f = 500/samplerate;

% make a phase accumulator / sawtooth
% from 0 .. 1
saw=mod1(saw + f-lp*0.1);

% square up the input signal
insq = sign(inl);

% make a product detector
ss = 2.0*saw-1
prod = insq*sign(ss);

% lowpass filter
c = 0.001
lp = (1-c)*lp + c*prod;

