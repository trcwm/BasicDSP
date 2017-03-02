# BasicDSP v2.0 24-2-2017
## Documentation

### Introduction

BasicDSP is an educational program that lets users experiment with simple Digital Signal Processing (DSP) algorithms. It compiles on Windows, OSX and Linux with QtCreator and the Qt framework.

![Screenshot of BasicDSP](examples/screenshot_pll.png?raw=true "Screenshot of BasicDSP")

For every input sample, a script is run to calculate an output sample. Input samples can come from a sound card or several built-in sources, including stereo .wav files. It features an oscilloscope and a spectrum analyzer

BasicDSP can be used to explore DSP algorithms, such as:
* Digital filters
* Digital PLL design
* Spectral analysis
* Non-linear functions (waveshapers, frequency mixers etc)
* Software-defined radio algorithms
* Sigma-delta noise shapers

### Script commands
* mod1(x) - returns the remainer of a division by one.
* sin1(x) - returns sin(2*pi*x)
* cos1(x) - returns cos(2*pi*x)
* sin(x) - returns sin(x), where x is in radians
* cos(x) - returns cos(x), where x is in radians
* tan(x) - returns the tangent of x, where x is in radians
* tanh(x) - returns the hyperbolic tangent of x, where x is in radians
* limit(x) - clamps x to be within -1..1
* round(x) - rounds x to the nearest integer
* sqrt(x) - returns the square root of x
* pow(x,y) - returns x^y
* abs(x) - returns the absolute value of x
* [atan2(y,x)](https://en.wikipedia.org/wiki/Atan2) - returns the arctangent of (y/x)
* sign(x) - returns 1 if x>=0 and -1 if x < 0.
* noise() - returns white noise with amplitude between -1 and 1.
* trunc(x) - rounds x toward zero, returning the nearest integral value that is not larger in magnitude than x.
* ceil(x) - rounds x upward, returning the smallest integral value that is not less than x.
* floor(x) - rounds x downward, returning the largest integral value that is not greater than x.

### Variables
* inl - left input channel
* inr - right input channel
* in - (left+right)/2
* outl - left output channel of sound card
* outr - right output channel of sound card
* out - writes to both left and right output channels of sound card
* samplerate - a read-only variable that contains the sample rate in Hz
