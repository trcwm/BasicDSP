/*

  Virtual machine for running basic DSP programs
  Niels A. Moseley 2016

  License: GPLv2

*/

#ifndef virtualmachine_h
#define virtualmachine_h

#include <stdint.h>
#include <vector>
#include "qmainwindow.h"
#include "portaudio.h"

#ifndef M_PI
#define M_PI 3.1415927
#endif

// instruction set of the VM
#define P_add 1
#define P_sub 2
#define P_mul 3
#define P_div 4

#define P_const 200

#define P_sin   100
#define P_cos   101
#define P_sin1  102
#define P_cos1  103
#define P_mod1  104
#define P_abs   105
#define P_round 106
#define P_sqrt  107
#define P_tan   108
#define P_tanh  109
#define P_pow   110

//#define P_print 102


// the following opcodes use the lower 16 bits for further identifying a variable or FIR
#define P_writevar 0x81000000
#define P_readvar  0x82000000
#define P_fir      0x83000000
#define P_biquad   0x84000000

// default variables
#define vidx_zero 0
#define vidx_samplerate 1
#define vidx_inl 2
#define vidx_inr 3
#define vidx_in  4
#define vidx_outl 5
#define vidx_outr 6
#define vidx_out  7

/** Virtual machine that executes BasicDSP programs.
    The VM runs in a different thread (due to PortAudio)
    and care must be taken to avoid data corruption
    caused by multi-threading.
*/
class VirtualMachine
{
public:
    VirtualMachine(QMainWindow *guiWindow);
    virtual ~VirtualMachine();

    /** load a program consisting of byte code */
    void loadProgram();

    /** start the execution of the program */
    bool start();

    /** stop the execution of the program */
    void stop();

    /** execute VM */
    void processSamples(float *inbuf,
                        float *outbuf,
                        uint32_t framesPerBuffer);

    /** get the current VU levels */
    void getVU(float &left, float &right)
    {
        left = m_leftLevel;
        right = m_rightLevel;
    }

protected:
    /** execute the program once */
    void executeProgram(float inLeft, float inRight, float &outLeft, float &outRight);

    /** calculate FIR output.
        returns the number of stack elements that are popped.
    */
    uint32_t execFIR(uint32_t n, float *stack);

    /** calculate Biquad section.
        returns the number of stack elements that are popped.
    */
    uint32_t execBiquad(uint32_t n, float *stack);

    QMainWindow *m_guiWindow;
    PaStream    *m_stream;
    double      m_sampleRate;
    float       m_leftLevel;
    float       m_rightLevel;

    union instruction_t
    {
        uint32_t icode;
        float    value;
    };

    struct variable_t
    {
        std::string name;
        float       value;
    };

    std::vector<instruction_t>   m_program; // array of program operations
    std::vector<variable_t>      m_vars;    // variables
};

#endif
