# BasicDSP v2.0 10-2-2017
## Documentation

### Introduction

BasicDSP is an educational program that lets users experiment with simple Digital Signal Processing (DSP) algorithms.

Note: there are still few missing features. Please be patient while we implement those and fix bugs.

### Commands
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
* sign(x) - returns 1 if x>=0 and -1 if x<0.

### Variables
* inl - left input channel
* inr - right input channel
* in - (left+right)/2
* outl - left output channel of sound card
* outr - right output channel of sound card
* out - writes to both left and right output channels of sound card
