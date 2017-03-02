/*

  Virtual machine for running basic DSP programs

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#include <QDebug>
#include <QMutexLocker>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <ostream>
#include <algorithm>
#include "virtualmachine.h"

int32_t VM::findVariableByName(const variables_t &vars, const std::string &name)
{
    size_t N = vars.size();
    size_t i=0;
    while(i<N)
    {
        if (vars[i].name == name)
            return (int32_t)i;
        i++;
    }
    return -1;
}

static int portaudioCallback(
        const void *inputBuffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData )
{
    (statusFlags);
    (timeInfo);

    /* Cast data passed through stream to our structure. */
    float *inbuf = (float*)inputBuffer;
    float *outbuf = (float*)outputBuffer;

    if (userData != 0)
    {
        VirtualMachine *machine = (VirtualMachine*)userData;
        machine->processSamples(inbuf, outbuf, framesPerBuffer);
    }

    return paContinue;
}



VirtualMachine::VirtualMachine(QMainWindow *guiWindow)
    : m_guiWindow(guiWindow),
      m_stream(0),
      m_runState(false)
{
    Pa_Initialize();

    for(uint32_t i=0; i<4; i++)
    {
        m_monitorVar[i] = NULL;
    }

    m_inDevice = Pa_GetDefaultInputDevice();
    m_outDevice = Pa_GetDefaultOutputDevice();
    m_sampleRate = 44100.0f;

    /* Allocate ring buffers for GUI I/O.

       The ring buffers are 16384 floats,
       which is approx 750ms of data at
       44100. The GUI thread must retrieve
       the data within this time.

       The number of floats must be a power
       of two!
    */
    for(uint32_t i=0; i<2; i++)
    {
        void *dataptr = new ring_buffer_data_t[32768];
        PaUtil_InitializeRingBuffer(&m_ringbuffer[i], sizeof(ring_buffer_data_t),
                                    32768, dataptr);
    }

    init();

    m_source = SRC_SOUNDCARD;
}

VirtualMachine::~VirtualMachine()
{
    Pa_Terminate();

    // de-allocate the ring buffer data
    for(uint32_t i=0; i<2; i++)
    {
        delete m_ringbuffer[i].buffer;
    }
}



void VirtualMachine::init()
{
    m_lout = 0;
    m_lin  = 0;
    m_rout = 0;
    m_rin  = 0;
    m_in   = 0;
    m_out  = 0;
    m_slider[0] = 0;
    m_slider[1] = 0;
    m_slider[2] = 0;
    m_slider[3] = 0;

    m_leftLevel = 0.0f;
    m_rightLevel = 0.0f;

    m_phaseaccu = 0.0f;
    m_freq = 0.0f;

    // flush the data in the ring buffers
    for(uint32_t i=0; i<2; i++)
    {
        PaUtil_FlushRingBuffer(&m_ringbuffer[i]);
    }
}

PaUtilRingBuffer* VirtualMachine::getRingBufferPtr(uint32_t ringBufID)
{
    if (ringBufID<2)
    {
        return &m_ringbuffer[ringBufID];
    }
    return NULL;
}

bool VirtualMachine::setMonitoringVariable(uint32_t ringBufID, uint32_t channel, const std::string &varname)
{
    QMutexLocker lock(&m_controlMutex);

    qDebug() << "setMonitoringVariable called";

    if (ringBufID > 1)
        return false;
    if (channel > 1)
        return false;

    int32_t idx = VM::findVariableByName(m_vars, varname);
    if (idx < 0)
    {
        // variable not found
        m_monitorVar[ringBufID*2 + channel] = NULL;
        return false;
    }

    qDebug() << "setMonitoringVariable " << varname.c_str();

    m_monitorVar[ringBufID*2 + channel] = &(m_vars[idx].value);
    return true;
}

bool VirtualMachine::hasAudioFile()
{
    QMutexLocker lock(&m_controlMutex);
    if (m_wavstreamer.isOK())
    {
        return true;
    }
    return false;
}

bool VirtualMachine::setAudioFile(const QString &filename)
{
    QMutexLocker lock(&m_controlMutex);
    if (m_wavstreamer.openFile(filename) != 0)
    {
        return false;
    }
    return true;
}

void VirtualMachine::loadProgram(const VM::program_t &program, const VM::variables_t &variables)
{
    QMutexLocker lock(&m_controlMutex);

    init();

    m_vars = variables;
    m_program = program;

    // find the lout, rout, lin, rin, in, out
    // variables.
    int32_t idx = VM::findVariableByName(m_vars, "inl");
    if (idx != -1) m_lin = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "inr");
    if (idx != -1) m_rin = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "outl");
    if (idx != -1) m_lout = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "outr");
    if (idx != -1) m_rout = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "out");
    if (idx != -1) m_out = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "in");
    if (idx != -1) m_in = &(m_vars[idx].value);

    // setup sliders
    idx = VM::findVariableByName(m_vars, "slider1");
    if (idx != -1) m_slider[0] = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "slider2");
    if (idx != -1) m_slider[1] = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "slider3");
    if (idx != -1) m_slider[2] = &(m_vars[idx].value);
    idx = VM::findVariableByName(m_vars, "slider4");
    if (idx != -1) m_slider[3] = &(m_vars[idx].value);

    // setup sample rate
    idx = VM::findVariableByName(m_vars, "samplerate");
    if (idx != -1)
    {
        m_vars[idx].value = m_sampleRate;
    }
}

void VirtualMachine::setupSoundcard(PaDeviceIndex inDevice, PaDeviceIndex outDevice, float sampleRate)
{
    qDebug() << "VirtualMachine::setupSoundcard";
    qDebug() << " in:   " << inDevice;
    qDebug() << " out:  " << outDevice;
    qDebug() << " rate: " << sampleRate;

    // stop the virtual machine
    // no mutex needed here, as it's handled in the
    // stop() function itself.

    stop();

    m_inDevice = inDevice;
    m_outDevice = outDevice;
    m_sampleRate = sampleRate;
}

bool VirtualMachine::start()
{
    qDebug() << "VirtualMachine::start()";

    QMutexLocker lock(&m_controlMutex);

    // check if portaudio is already running
    if (m_stream != 0)
    {
        m_runState = false;
        Pa_AbortStream(m_stream);
        Pa_CloseStream(m_stream);
    }

    m_leftLevel = 0.0f;
    m_rightLevel = 0.0f;

    const double sampleRate = m_sampleRate;
    const uint32_t framesPerBuffer = 0;
    PaSampleFormat sampleFormat = paFloat32;
    PaStreamParameters inputParams;
    PaStreamParameters outputParams;

    memset(&inputParams, 0, sizeof(inputParams));
    inputParams.device = m_inDevice;
    inputParams.suggestedLatency = 0.2f;
    inputParams.channelCount = 2;
    inputParams.suggestedLatency = 0.2;
    inputParams.sampleFormat = sampleFormat;

    memset(&outputParams, 0, sizeof(outputParams));
    outputParams.device = m_outDevice;
    outputParams.suggestedLatency = 0.2f;
    outputParams.channelCount = 2;
    outputParams.suggestedLatency = 0.2;
    outputParams.sampleFormat = sampleFormat;

    PaError error = Pa_OpenStream(
                &m_stream,
                &inputParams,
                &outputParams,
                sampleRate,
                framesPerBuffer,
                0,
                portaudioCallback,
                this);

    if (error == paNoError)
    {
        error = Pa_StartStream(m_stream);
        if (error == paNoError)
        {
            qDebug() << "Stream started!";
            m_runState = true;
            return true;
        }
    }

    qDebug() << "Portaudio: " << Pa_GetErrorText(error) << "\n";

    return false;
}

void VirtualMachine::stop()
{
    QMutexLocker lock(&m_controlMutex);

    if (m_stream != 0)
    {
        Pa_AbortStream(m_stream);
        Pa_CloseStream(m_stream);
        m_stream = 0;
    }
    m_leftLevel = 0.0f;
    m_rightLevel = 0.0f;

    m_runState = false;
}

void VirtualMachine::setSlider(uint32_t id, float value)
{
    QMutexLocker lock(&m_controlMutex);
    if (id < 4)
    {
        if (m_slider[id] != 0)
        {
            *m_slider[id]=value;
        }
    }
}

void VirtualMachine::setSource(src_t source)
{
    QMutexLocker locker(&m_controlMutex);
    m_source = source;
}

void VirtualMachine::setFrequency(double Hz)
{
    QMutexLocker locker(&m_controlMutex);
    m_freq = Hz;
}

void VirtualMachine::processSamples(float *inbuf, float *outbuf,
                                    uint32_t framesPerBuffer)
{
    // as this is a time-critical function that is
    // called by the audio subsystem
    // blocking it is not a good idea
    // therefore, we'll try to lock the mutex
    // but if that fails, we return a muted buffer
    //

    bool success = m_controlMutex.tryLock();
    if (!success)
    {
        for(uint32_t i=0; i<framesPerBuffer; i++)
        {
            *outbuf++ = 0.0f;
            *outbuf++ = 0.0f;
        }
        return;
    }

    // todo: make multiplier respect the frames per buffer
    // so we get block-size independent VU meter behaviour.
    m_leftLevel *= 0.9f;
    m_rightLevel *= 0.9f;

    float wavBuffer[2];
    for(uint32_t i=0; i<framesPerBuffer; i++)
    {
        float left;
        float right;

        switch(m_source)
        {
        default:
        case SRC_SOUNDCARD:
            left = *inbuf++;
            right = *inbuf++;
            break;
        case SRC_WAV:           
            m_wavstreamer.fillBuffer(wavBuffer, 1);
            left = wavBuffer[0];
            right = wavBuffer[1];
            break;
        case SRC_NOISE:
            left = -1.0f+2.0f*static_cast<float>(rand())/RAND_MAX;
            right = -1.0f+2.0f*static_cast<float>(rand())/RAND_MAX;
            break;
        case SRC_SINE:
            left = cos(2.0f*3.1415927f*m_phaseaccu);
            right = left;
            m_phaseaccu += (m_freq / m_sampleRate);
            if (m_phaseaccu > 1.0f)
            {
                m_phaseaccu -= 1.0f;
            }
            else if (m_phaseaccu < 0.0f)
            {
                // safety check
                m_phaseaccu = 0.0f;
            }
            break;
        case SRC_QUADSINE:
            left = cos(2.0f*3.1415927f*m_phaseaccu);
            right = sin(2.0f*3.1415927f*m_phaseaccu);
            m_phaseaccu += (m_freq / m_sampleRate);
            if (m_phaseaccu > 1.0f)
            {
                m_phaseaccu -= 1.0f;
            }
            else if (m_phaseaccu < -1.0f)
            {
                m_phaseaccu += 1.0f;
            }
            break;
        case SRC_IMPULSE:
            left = 0.0f;
            right = 0.0f;
        }

        float left_abs = fabs(left);
        float right_abs = fabs(right);

        if (left_abs > m_leftLevel)
        {
            m_leftLevel = left_abs;
        }
        if (right_abs > m_rightLevel)
        {
            m_rightLevel = right_abs;
        }
        executeProgram(left, right, outbuf[i<<1], outbuf[(i<<1)+1]);

        ring_buffer_data_t scope;
        ring_buffer_data_t spectrum;

        if (m_monitorVar[0] != NULL)
        {
            scope.s1 = *m_monitorVar[0];
        }
        else
        {
            scope.s1 = 0;
        }

        if (m_monitorVar[1] != NULL)
        {
            scope.s2 = *m_monitorVar[1];
        }
        else
        {
            scope.s2 = 0;
        }

        if (m_monitorVar[2] != NULL)
        {
            spectrum.s1 = *m_monitorVar[2];
        }
        else
        {
            spectrum.s1 = 0;
        }

        if (m_monitorVar[3] != NULL)
        {
            spectrum.s2 = *m_monitorVar[3];
        }
        else
        {
            spectrum.s2 = 0;
        }

        PaUtil_WriteRingBuffer(&m_ringbuffer[0], &scope, 1);
        PaUtil_WriteRingBuffer(&m_ringbuffer[1], &spectrum, 1);
    }
    m_controlMutex.unlock();
}

uint32_t VirtualMachine::execFIR(uint32_t n, float *stack)
{
    return 0;
}

uint32_t VirtualMachine::execBiquad(uint32_t n, float *stack)
{
    return 0;
}

void VirtualMachine::executeProgram(float inLeft, float inRight, float &outLeft, float &outRight)
{
    const size_t instructions = m_program.size();
    size_t pc = 0;    // program counter
    size_t sp = 0;    // stack pointer
    float stack[2048];

    // check if we have a program ..
    // or if we're not running...
    if ((instructions == 0) || (!m_runState))
    {
        outLeft = 0.0f;
        outRight = 0.0f;
        return;
    }

    // setup input signal
    if (m_in != 0)
    {
        *m_in = (inLeft + inRight) / 2.0f;
    }
    if (m_lin != 0)
    {
        *m_lin = inLeft;
    }
    if (m_rin != 0)
    {
        *m_rin = inRight;
    }

    while(pc < instructions)
    {
        VM::instruction_t instruction = m_program[pc++];
        if (instruction.icode & 0x80000000)
        {
            // special instruction with additional parameter
            uint32_t n = instruction.icode & 0xFFFF;
            switch(instruction.icode & 0xff000000)
            {
            case P_readvar: // push
                stack[sp++] = m_vars[n].value;
                break;
            case P_writevar:// pop
                m_vars[n].value = stack[--sp];
                break;
            case P_fir:
                sp-=execFIR(n, stack+sp);
                break;
            case P_biquad:
                sp-=execBiquad(n, stack+sp);
                break;
            default:
                // TODO: produce error
                break;
            }
        }
        else
        {
            switch(instruction.icode)
            {
            case P_add:
                sp--;
                stack[sp-1]+=stack[sp];
                break;
            case P_sub:
                sp--;
                stack[sp-1]=stack[sp-1]-stack[sp];
                //qDebug() << stack[sp-1];
                break;
            case P_mul:
                sp--;
                stack[sp-1]*=stack[sp];
                break;
            case P_div:
                sp--;
                stack[sp-1]/=stack[sp];
                break;
            case P_neg:
                stack[sp-1]=-stack[sp-1];
                break;
            case P_sin:
                stack[sp-1]=sin(stack[sp-1]);
                break;
            case P_tan:
                stack[sp-1]=tan(stack[sp-1]);
                break;
            case P_tanh:
                stack[sp-1]=tanh(stack[sp-1]);
                break;
            case P_cos:
                stack[sp-1]=cos(stack[sp-1]);
                break;
            case P_sin1:
                stack[sp-1]=sin(2.0f*M_PI*stack[sp-1]);
                break;
            case P_cos1:
                stack[sp-1]=cos(2.0f*M_PI*stack[sp-1]);
                break;
            case P_literal:
                stack[sp++]=m_program[pc].value;
                pc++;
                break;
            //case P_print:
            //  printf("%f\n",stack[--sp]);
            //    break;
            case P_mod1:
                stack[sp-1]=stack[sp-1]-(int)stack[sp-1];
                break;
            case P_abs:
                stack[sp-1]=fabs(stack[sp-1]);
                break;
            case P_sqrt:
                stack[sp-1]=sqrt(stack[sp-1]);
                break;
            case P_round:
                //sp--;
                //if (stack[sp]!=0.0f)
                //    stack[sp-1]=stack[sp]*round(stack[sp-1]/stack[sp]);
                stack[sp-1]=round(stack[sp-1]);
                break;
            case P_pow:
                sp--;
                stack[sp-1]=pow(stack[sp-1],stack[sp]);
                break;
            case P_limit:
                stack[sp-1]=std::min(stack[sp-1],1.0f);
                stack[sp-1]=std::max(stack[sp-1],-1.0f);
                break;
            case P_atan2:
                sp--;
                stack[sp-1]=atan2(stack[sp-1],stack[sp]);
                break;
            case P_sign:
                if (stack[sp-1] >= 0.0f)
                    stack[sp-1]=1.0f;
                else
                    stack[sp-1]=-1.0f;
                break;
            case P_noise:
                stack[sp++]=-1.0f+2.0f*static_cast<float>(rand())/RAND_MAX;
                break;
            case P_trunc:
                stack[sp-1] = std::trunc(stack[sp-1]);
                break;
            case P_ceil:
                stack[sp-1] = std::ceil(stack[sp-1]);
                break;
            case P_floor:
                stack[sp-1] = std::floor(stack[sp-1]);
                break;
            default:
                // TODO: produce error
                break;
            }
        }
        if (sp>2044)
        {
            // stack overflow!
            return;
        }
    }

    if (m_out != 0)
    {
        outLeft = *m_out;
        outRight = *m_out;
    }
    else
    {
        if (m_lout != 0)
        {
            outLeft = *m_lout;
        }
        else
        {
            outLeft = 0.0f;
        }
        if (m_rout != 0)
        {
            outRight = *m_rout;
        }
        else
        {
            outRight = 0.0f;
        }
    }
}

void VirtualMachine::dump(std::ostream &s)
{
    QMutexLocker lock(&m_controlMutex);

    s << "-- VIRTUAL MACHINE PROGRAM --\n\n";
    size_t N = m_program.size();
    for(size_t i=0; i<N; i++)
    {
        uint32_t n = m_program[i].icode & 0xFFFF; // variable index)
        if (m_program[i].icode & 0x80000000)
        {
            switch(m_program[i].icode & 0xff000000)
            {
            case P_readvar:
                s << "READ " << m_vars[n].name.c_str() << "\n";
                break;
            case P_writevar:
                s << "WRITE " << m_vars[n].name.c_str() << "\n";
                break;
            default:
                s << "UNKNOWN\n";
                break;
            }
        }
        else
        {
            switch(m_program[i].icode)
            {
            case P_add:
                s << "ADD\n";
                break;
            case P_sub:
                s << "SUB\n";
                break;
            case P_mul:
                s << "MUL\n";
                break;
            case P_div:
                s << "DIV\n";
                break;
            case P_literal:
                s << "LOAD " << m_program[i+1].value << "\n";
                i++;
                break;
            case P_sin:
                s << "SIN\n";
                break;
            case P_cos:
                s << "COS\n";
                break;
            case P_sin1:
                s << "SIN1\n";
                break;
            case P_cos1:
                s << "COS1\n";
                break;
            case P_mod1:
                s << "MOD1\n";
                break;
            case P_abs:
                s << "ABS\n";
                break;
            case P_tan:
                s << "TAN\n";
                break;
            case P_tanh:
                s << "TANH\n";
                break;
            case P_pow:
                s << "POW\n";
                break;
            case P_sqrt:
                s << "SQRT\n";
                break;
            case P_round:
                s << "ROUND\n";
                break;
            case P_limit:
                s << "LIMIT\n";
                break;
            case P_atan2:
                s << "ATAN2\n";
                break;
            case P_noise:
                s << "NOISE\n";
                break;
            case P_trunc:
                s << "TRUNC\n";
                break;
            case P_ceil:
                s << "CEIL\n";
                break;
            case P_floor:
                s << "FLOOR\n";
                break;
            default:
                s << "UNKNOWN\n";
                break;
            }
        }
    }
}
