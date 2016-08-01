/*

  Virtual machine for running basic DSP programs
  Niels A. Moseley 2016

  License: GPLv2

*/

#ifndef virtualmachine_h
#define virtualmachine_h

#include <stdint.h>
#include <vector>
#include <QMutex>
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
#define P_neg 5

#define P_literal 200

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
#define P_limit 111

//#define P_print 102


// the following opcodes use the lower 16 bits for further identifying a variable or FIR
#define P_writevar 0x81000000
#define P_readvar  0x82000000
#define P_fir      0x83000000
#define P_biquad   0x84000000

namespace VM
{
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

    typedef std::vector<instruction_t> program_t;
    typedef std::vector<variable_t>    variables_t;

    /** find a variable by name. returns -1 if not found */
    int32_t findVariableByName(const variables_t &vars, const std::string &name);
}

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
    void loadProgram(const VM::program_t &program, const VM::variables_t &variables);

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
        QMutexLocker locker(&m_controlMutex);
        left = m_leftLevel;
        right = m_rightLevel;
    }

    /** set the value of a slider */
    void setSlider(uint32_t id, float value);

    /** set the source input */
    enum src_t {SRC_SOUNDCARD, SRC_NOISE, SRC_SINE, SRC_QUADSINE, SRC_WAV, SRC_IMPULSE};
    void setSource(src_t source);

    /** dump the (human readable) VM program to an output stream */
    void dump(std::ostream &s);

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

    bool        m_runState;

    QMutex      m_controlMutex;

    VM::program_t       m_program;
    VM::variables_t     m_vars;

    src_t   m_source;

    float   *m_lout;
    float   *m_lin;
    float   *m_rout;
    float   *m_rin;
    float   *m_in;
    float   *m_out;

    float   *m_slider[4];
};

#endif
