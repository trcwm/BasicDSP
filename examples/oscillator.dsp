%
% A simple tunable sinusoidal oscillator.
% The frequency is set using slider 1
% The amplitude is set using slider 2
%
% To display signals in the oscilloscope,
% press 'scope' button and fill in 'out'
% or 'saw' in one of the channel boxes.
% 
% Press 'spectrum' button and fill in
% 'out' or 'saw' in one of the channel boxes.

% set the frequency 
f = slider1/2;

% make a phase accumulator / sawtooth
% from 0 .. 1
saw=mod1(saw + f);

% output a sine wave 
out=sin1(saw)*slider2;
