/*

  Virtual machine for running basic DSP programs
  Niels A. Moseley 2016

  License: GPLv2

*/

#ifndef virtualmachine_h
#define virtualmachine_h

#include <stdint.h>
#include "qmainwindow.h"
#include "portaudio.h"

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
    QMainWindow *m_guiWindow;
    PaStream    *m_stream;
    double      m_sampleRate;
    float       m_leftLevel;
    float       m_rightLevel;
};

#endif
