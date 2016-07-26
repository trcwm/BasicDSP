/*

  Virtual machine for running basic DSP programs
  Niels A. Moseley 2016

  License: GPLv2

*/

#include <QDebug>
#include <stdint.h>
#include "virtualmachine.h"

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
      m_stream(0)
{
    Pa_Initialize();

    // for now, dump the devices to output
    PaDeviceIndex count = Pa_GetDeviceCount();
    for(PaDeviceIndex idx=0; idx<count; idx++)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(idx);
        if (info != 0)
        {
            qDebug() << "-----------------------------------------";
            qDebug() << idx << " -> " << info->name;
            qDebug() << " Inputs: " << info->maxInputChannels;
            qDebug() << " Outputs: " << info->maxOutputChannels;

            PaHostApiIndex hindex = info->hostApi;
            const PaHostApiInfo *hinfo = Pa_GetHostApiInfo(hindex);
            qDebug() << " API: " << hinfo->name;
        }
    }
}

VirtualMachine::~VirtualMachine()
{
    Pa_Terminate();
}

void VirtualMachine::loadProgram()
{
    // make room for default variables
    //
    //
    m_vars.resize(6);
    m_vars[vidx_zero].name = "zero";
    m_vars[vidx_zero].value = 0.0f;
    m_vars[vidx_samplerate].name = "samplerate";
    m_vars[vidx_samplerate].value = 44100.0f;
    m_vars[vidx_inl].name = "inl";
    m_vars[vidx_inl].value = 0.0f;
    m_vars[vidx_inr].name = "inr";
    m_vars[vidx_inr].value = 0.0f;
    m_vars[vidx_in].name = "in";
    m_vars[vidx_in].value = 0.0f;
    m_vars[vidx_outl].name = "outl";
    m_vars[vidx_outl].value = 0.0f;
    m_vars[vidx_outr].name = "outr";
    m_vars[vidx_outr].value = 0.0f;
    m_vars[vidx_out].name = "out";
    m_vars[vidx_out].value = 0.0f;
}

bool VirtualMachine::start()
{
    // check if portaudio is already running
    if (m_stream != 0)
    {
        Pa_AbortStream(m_stream);
        Pa_CloseStream(m_stream);
    }

    m_leftLevel = 0.0f;
    m_rightLevel = 0.0f;

    const double sampleRate = 48000;
    const uint32_t framesPerBuffer = 0;
    PaSampleFormat sampleFormat = paFloat32;
    PaStreamParameters inputParams;
    PaStreamParameters outputParams;

    memset(&inputParams, 0, sizeof(inputParams));
    inputParams.device = 9;
    inputParams.channelCount = 2;
    inputParams.sampleFormat = sampleFormat;

    memset(&outputParams, 0, sizeof(outputParams));
    outputParams.device = 8;
    outputParams.channelCount = 2;
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
            return true;
        }
    }

    qDebug() << "Portaudio: " << Pa_GetErrorText(error) << "\n";

    return false;
}

void VirtualMachine::stop()
{
    if (m_stream != 0)
    {
        Pa_AbortStream(m_stream);
        Pa_CloseStream(m_stream);
        m_stream = 0;
    }
    m_leftLevel = 0.0f;
    m_rightLevel = 0.0f;
}

void VirtualMachine::processSamples(float *inbuf, float *outbuf,
                                    uint32_t framesPerBuffer)
{
    static float accu = 0.0f;

    m_leftLevel *= 0.9f;
    m_rightLevel *= 0.9f;

    for(uint32_t i=0; i<framesPerBuffer; i++)
    {
        float left = *inbuf++;
        float right = *inbuf++;
        float left_abs = fabs(left*1000.0f);
        float right_abs = fabs(right*1000.0f);

        if (left_abs > m_leftLevel)
        {
            m_leftLevel = left_abs;
        }
        if (right_abs > m_rightLevel)
        {
            m_rightLevel = right_abs;
        }

        //*outbuf++ = sin(3.1415927f*accu);
        //*outbuf++ = sin(3.1415927f*accu);
        *outbuf++ = 0;
        *outbuf++ = 0;

        accu += 0.05f;
        if (accu > 2.0f)
        {
            accu -= 2.0f;
        }
    }
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
    const uint32_t instructions = m_program.size();
    uint32_t pc = 0;    // program counter
    uint32_t sp = 0;    // stack pointer
    float stack[2048];

    m_vars[vidx_inl].value = inLeft;
    m_vars[vidx_inr].value = inRight;
    m_vars[vidx_in].value = (inLeft+inRight)/2.0f;

    while(pc < instructions)
    {
        instruction_t instruction = m_program[pc++];
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
                stack[sp-1]-=stack[sp];
                break;
            case P_mul:
                sp--;
                stack[sp-1]*=stack[sp];
                break;
            case P_div:
                sp--;
                stack[sp-1]/=stack[sp];
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
                stack[sp-1]=sin(2*M_PI*stack[sp-1]);
                break;
            case P_cos1:
                stack[sp-1]=cos(2*M_PI*stack[sp-1]);
                break;
            case P_const:
                stack[sp++]=m_program[pc].value;
                pc++;
                break;
            //case P_print:
            //    printf("%f\n",stack[--sp]);
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
                sp--;
                if (stack[sp]!=0.0f)
                    stack[sp-1]=stack[sp]*round(stack[sp-1]/stack[sp]);
                break;
            case P_pow:
                sp--;
                stack[sp-1]=pow(stack[sp-1],stack[sp]);
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

    bool m_stereoOut = true;
    if (m_stereoOut)
    {
        outLeft  = m_vars[vidx_outl].value;
        outRight = m_vars[vidx_outr].value;
    }
    else
    {
        outLeft  = m_vars[vidx_out].value;
        outRight = m_vars[vidx_out].value;
    }

    //TODO: setup Portaudio non-blocking ring buffers
    // to communicate variables to the GUI thread
}
