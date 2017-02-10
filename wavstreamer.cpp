/********************************************************************

  Wav streamer rev3.
  Code by N.A. Moseley
  Copyright 2006-2007

  Note: only stereo files using PCM or IEEE floats are supported!
  supports 16,24,32 bit PCM and 32 bit float.
  stereo only!!!
  License: GPLv2

  rev3: first working version
  rev4: converted to unicode filenames & wxwidgets FileStream
********************************************************************/

#include <QFile>
#include "wavstreamer.h"

#define TEMPBUFFERSIZE 65536

WavStreamer::WavStreamer() :
    m_waveStream(NULL),
    sampleIndex(0),
    tempBuffer(NULL),
    m_isOK(false)
{

}

WavStreamer::~WavStreamer()
{
    if (m_waveStream != NULL)
        delete m_waveStream;  // destructor automatically closes the file.

    if (tempBuffer != NULL)
        delete[] static_cast<char*>(tempBuffer);
}

int32_t WavStreamer::openFile(const QString &filename)
{
    if (m_waveStream != 0)
    {
        delete m_waveStream;
        m_waveStream = 0;
    }

    if (m_file.isOpen())
    {
        m_file.close();
    }

    m_isOK = false;

    // try to open the file
    m_file.setFileName(filename);
    if (m_file.open(QIODevice::ReadOnly) == false)
    {
        return -2;  // error opening file
    }

    m_waveStream = new QDataStream(&m_file);

    // check for RIFF file & size
    size_t bytes = m_waveStream->readRawData(m_chunkType, 4);
    if ((bytes != 4) || (strncmp(m_chunkType, "RIFF", 4)!=0))
    {
        delete m_waveStream;
        m_waveStream = NULL;
        m_file.close();
        return -1;
    }

    bytes = m_waveStream->readRawData((char*)&m_chunkSize, 4);
    if (bytes != 4)
    {
        delete m_waveStream;
        m_waveStream = NULL;
        m_file.close();
        return -1;
    }

    bytes = m_waveStream->readRawData((char*)&m_chunkType, 4);
    if ((bytes != 4) && (strncmp(m_chunkType, "WAVE", 4)!=0))
    {
        delete m_waveStream;
        m_waveStream = NULL;
        m_file.close();
        return -1;
    }

    m_riffSize = m_chunkSize + 8; // size of RIFF chunk + 4-byte ID + 4-byte chunksize.
    if (!findChunk("fmt "))
    {
        // error, format chunk not found!
        delete m_waveStream;
        m_waveStream = NULL;
        m_file.close();
        return -1;
    }

    // now, read the format chunk
    bytes = m_waveStream->readRawData((char*)&m_waveFormat, sizeof(WavFormatChunk));
    if (m_waveFormat.wFormatTag == 65534)  // WAVE_FORMAT_EXTENSIBLE case..
    {
        m_waveFormat.wFormatTag = 1;  // treat as PCM data, which should be the same when we have only 2 channels.
    }

    if (((m_waveFormat.wFormatTag != 3) && (m_waveFormat.wFormatTag != 1)) || (m_waveFormat.wChannels != 2))
    {
        delete m_waveStream;
        m_waveStream = NULL;
        m_file.close();
        return -1;  // wrong format!
    }
    // formats smaller than 16 bits are not supported!
    if (m_waveFormat.wBitsPerSample < 16)
    {
        delete m_waveStream;
        m_waveStream = NULL;
        m_file.close();
        return -1;
    }

    // if there are additional bytes in the format chunk, skip them.
    // important in the WAVE_FORMAT_EXTENSIBLE case!
    if (sizeof(WavFormatChunk) != m_chunkSize)
    {
        m_waveStream->skipRawData(m_chunkSize - sizeof(WavFormatChunk));
    }

    // now, search for the audio data and set the file offset pointers
    if (!findChunk("data"))
    {
        delete m_waveStream;
        m_waveStream = NULL;
        m_file.close();
        return -1;
    }

    m_playStart  = m_waveStream->device()->pos();
    m_playOffset = m_playStart;
    m_playEnd    = m_playStart + m_chunkSize;

    // allocate the correct temporary buffer.
    if (tempBuffer!=NULL) delete[] static_cast<char*>(tempBuffer);
    tempBuffer = static_cast<void*>(new char[m_waveFormat.wBitsPerSample*TEMPBUFFERSIZE*2/8]);

    // don't forget to actually read the data
    readRawData(TEMPBUFFERSIZE);
    sampleIndex = 0;

    m_filename = filename;
    m_isOK = true;
    return 0;
}

bool WavStreamer::findChunk(const char ID[4])
{
    if (m_waveStream == 0)
    {
        return false;
    }

    size_t bytesRead = m_waveStream->device()->pos();
    m_chunkSize = 0;

    // iterate until we get a format chunk..
    while ((m_riffSize > bytesRead) && (strncmp(m_chunkType, ID, 4)!=0))
    {
        // skip the size of the chunk (except for the first RIFF chunk!)
        m_waveStream->skipRawData(m_chunkSize);
        bytesRead += m_chunkSize;

        // read type of next chunk
        size_t bytes = m_waveStream->readRawData(m_chunkType, 4);
        bytesRead += bytes;

        bytes = m_waveStream->readRawData((char*)&m_chunkSize, 4);
        bytesRead += bytes;
    }

    if (m_riffSize < bytesRead)
    {
        return false;
    }

    return true;
}

uint32_t WavStreamer::readRawData(uint32_t requestedSamples)
{
    if (m_waveStream == 0)
    {
        return 0;
    }

    uint32_t bytes_per_sample;
    switch(m_waveFormat.wBitsPerSample)
    {
    case 16:
        bytes_per_sample = 2;
        break;
    case 24:
        bytes_per_sample = 3;
        break;
    case 32:
        bytes_per_sample = 4;
        break;
    default:
        return -1;  // error, wrong format!
    }

    int bytes_to_read = requestedSamples * 2 * bytes_per_sample;  // RequestedSamples is in 'stereo', so 2 real samples.
    int offset = 0;
    while(bytes_to_read>0)
    {
        if (bytes_to_read > (m_playEnd - m_playOffset))  // check for wrap-around
        {
            int readAmount = (m_playEnd - m_playOffset);
            uint32_t bytesRead = m_waveStream->readRawData(static_cast<char*>(tempBuffer) + offset, readAmount);
            if (bytesRead != readAmount)
            {
                // FIXME: file read error
                m_isOK = false;
                return 0;
            }

            m_waveStream->device()->seek(m_playStart);

            offset += readAmount;
            m_playOffset = m_playStart;
            bytes_to_read -= readAmount;
        }
        else
        {
            uint32_t bytesRead = m_waveStream->readRawData(static_cast<char*>(tempBuffer) + offset, bytes_to_read);
            if (bytesRead != bytes_to_read)
            {
                // FIXME: file read error
                m_isOK = false;
                return 0;
            }

            m_playOffset += bytes_to_read;
            bytes_to_read = 0;
        }
    }
    sampleIndex = 0;
    return requestedSamples;
}

void WavStreamer::fillBuffer(float *stereoBuffer, uint32_t stereoSamples)
{
    if (m_waveStream==NULL)
    {
        // clear the buffer if we have no file to read
        // Note: this is compatible with the IEEE 756 floating-point format!
        memset(stereoBuffer, 0, sizeof(float)*2*stereoSamples);
        return;
    }
    else
    {
        int monoSamples = stereoSamples * 2;
        if (m_waveFormat.wFormatTag == 1) // PCM
        {
            switch(m_waveFormat.wBitsPerSample)
            {
            case 16:
                {
                    int16_t *ptr = static_cast<short*>(tempBuffer);
                    for(int i=0; i<monoSamples; i++)
                    {
                        if (sampleIndex > (TEMPBUFFERSIZE*2-1)) readRawData(TEMPBUFFERSIZE);
                        stereoBuffer[i] = static_cast<float>(ptr[sampleIndex]) / 32768.0f;
                        sampleIndex++;
                    }
                }
                break;
            case 24:
                {
                    uint8_t *ptr = static_cast<uint8_t*>(tempBuffer);
                    long v = 0;
                    int index = sampleIndex * 3;
                    for(int i=0; i<monoSamples; i++)
                    {
                        if (sampleIndex > (TEMPBUFFERSIZE*2-1))
                        {
                            readRawData(TEMPBUFFERSIZE);
                            index = 0;
                        }
                        v = static_cast<uint32_t>(ptr[index]) << 8;
                        v |= static_cast<uint32_t>(ptr[index+1]) << 16;
                        uint8_t top = static_cast<uint32_t>(ptr[index+2]);
                        v |= (top << 24);
                        if (top >= 0x80)
                        {
                            // signed!
                            v |= 0xFF000000;
                        }

                        index += 3;
                        stereoBuffer[i] = static_cast<float>(v) / 2147483648.0f;
                        sampleIndex++;
                    }
                }
                break;
            case 32:
                {
                    int32_t *ptr = static_cast<int*>(tempBuffer);
                    for(int i=0; i<monoSamples; i++)
                    {
                        if (sampleIndex > (TEMPBUFFERSIZE*2-1))
                        {
                            readRawData(TEMPBUFFERSIZE);
                        }
                        stereoBuffer[i] = static_cast<float>(ptr[sampleIndex]) / 2147483648.0f;
                        sampleIndex++;
                    }
                }
                break;
            }
        }
        else if (m_waveFormat.wFormatTag == 3)
        {
            // uncompressed 32-bit float data
            float *ptr = static_cast<float*>(tempBuffer);
            for(int i=0; i<monoSamples; i++)
            {
                if (sampleIndex > (TEMPBUFFERSIZE*2-1))
                {
                    readRawData(TEMPBUFFERSIZE);
                }
                stereoBuffer[i] = ptr[sampleIndex]; // FIXME: scaling??
                sampleIndex++;
            }
        }
    }
}

bool WavStreamer::getFormat(WavFormatChunk &output) const
{
    if (m_waveStream == NULL)
    {
        return false;
    }

    output = m_waveFormat;
    return true;
}
