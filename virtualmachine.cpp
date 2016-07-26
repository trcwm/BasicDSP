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

        *outbuf++ = sin(3.1415927f*accu);
        *outbuf++ = sin(3.1415927f*accu);
        accu += 0.05f;
        if (accu > 2.0f)
        {
            accu -= 2.0f;
        }
    }
}
