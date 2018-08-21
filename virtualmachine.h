/*

  Virtual machine for running basic DSP programs

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#ifndef virtualmachine_h
#define virtualmachine_h

#include <stdint.h>
#include <vector>
#include <QMutex>
#include "varinfo.h"
#include "qmainwindow.h"
#include "portaudio.h"
#include "portaudio_helper.h"
#include "pa_ringbuffer.h"
#include "wavstreamer.h"

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
#define P_atan2 112
#define P_sign  113
#define P_noise 114
#define P_trunc 115
#define P_ceil  116
#define P_floor 117
#define P_choose 118

// the following opcodes use the lower 16 bits for further identifying a variable or FIR
#define P_writevar      0x81000000
#define P_readvar       0x82000000
#define P_fir           0x83000000
#define P_biquad        0x84000000
#define P_writedelay    0x85000000
#define P_readdelay     0x86000000

namespace VM
{
    union instruction_t
    {
        uint32_t icode;
        float    value;
    };

    typedef std::vector<instruction_t> program_t;
    typedef std::vector<varInfo>       variables_t;

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

    /** returns true if the virtual machine is running */
    bool isRunning() const
    {
        return m_runState;
    }

    /** execute VM */
    void processSamples(const float *inbuf,
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

    /** set the frequency for the sine or quadsine generator in Hertz */
    void setFrequency(double Hz);

    /** dump the (human readable) VM program to an output stream */
    void dump(std::ostream &s);

    /** set monitoring variables for ring buffer */
    bool setMonitoringVariable(uint32_t ringBufID, uint32_t channel, const std::string &varname);

    /** set the audio file for the wav streamer */
    bool setAudioFile(const QString &filename);

    /** returns true if there is a valid audio file to use */
    bool hasAudioFile();

    /** get a pointer to one of the two ring buffers
        to allow the reading of data by the GUI thread */
    PaUtilRingBuffer* getRingBufferPtr(uint32_t ringBufID);

    /** set the soundcard device parameters */
    void setupSoundcard(PaDeviceIndex inDevice, PaDeviceIndex outDevice,
                        float sampleRate);

    PaDeviceIndex getInputDevice() const
    {
        return m_inDevice;
    }

    PaDeviceIndex getOutputDevice() const
    {
        return m_outDevice;
    }

    float getSamplerate() const
    {
        return m_sampleRate;
    }

    struct ring_buffer_data_t
    {
        float s1;
        float s2;
    };

protected:
    /** initialize internal pointers */
    void init();

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

    PaDeviceIndex m_inDevice;
    PaDeviceIndex m_outDevice;
    double      m_sampleRate;   // the current sample rate in Hz

    float       m_leftLevel;    // the left channel VU level
    float       m_rightLevel;   // the right channel VU level
    bool        m_runState;     // true if VM is running a program

    QMutex      m_controlMutex; // mutex to synchronize GUI and VM threads

    VM::program_t   m_program;  // VM byte code
    VM::variables_t m_vars;     // VM program variables

    src_t   m_source;           // selected input source

    // the following pointers are variables in
    // m_vars, or NULL if the variable does not exist
    // in the current VM program
    float   *m_lout;            // pointer to left OUT variable
    float   *m_lin;             // pointer to left IN variable
    float   *m_rout;            // pointer to right OUT variable
    float   *m_rin;             // pointer to right IN variable
    float   *m_in;              // pointer to mono IN variable
    float   *m_out;             // pointer to mono OUT variable
    float   *m_slider[4];       // pointers to slider variables

    float   m_freq;             // sine or quadsine frequency (in Hz)
    float   m_phaseaccu;        // phase accumulator [0..1) for frequency generator

    // variables to send to spectrum & scope displays
    // can be NULL if nothing is selected
    float   *m_monitorVar[4];

    // thread-safe ring buffers for GUI I/O
    PaUtilRingBuffer m_ringbuffer[2];

    // handles audio streaming from .wav files
    WavStreamer m_wavstreamer;
};

#endif
