/********************************************************************

  Wav streamer rev4.
  Code by N.A. Moseley
  Copyright 2006-2017

  Note: only stereo files using PCM or IEEE floats are supported!
  supports 16,24,32 bit PCM and 32 bit float.
  stereo only!!!
  License: GPLv2

********************************************************************/

#ifndef wavstreamer_h
#define wavstreamer_h

#include <QFile>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <QDataStream>

#include <stdint.h>
// ----------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint16_t       wFormatTag;  // should be '1' for uncompressed PCM, or '3' for uncompressed IEEE float data.
    uint16_t       wChannels;
    uint32_t       dwSamplesPerSec;
    uint32_t       dwAvgBytesPerSec;
    uint16_t       wBlockAlign;
    uint16_t       wBitsPerSample;

    /* Note: there may be additional fields here, depending upon wFormatTag. */

} WavFormatChunk;

//typedef struct {
//  char           chunkID[4];  // 'data'
//  long           chunkSize;

//  unsigned char  *waveformData;
//} WavDataChunk;

#pragma pack(pop)

// ----------------------------------------------------------

class WavStreamer
{
public:
    WavStreamer();
    virtual ~WavStreamer();

    /** Opens a file for reading
        @return error code. 0 = ok, -1 = invalid format, -2 = cannot open file
    */
    int32_t openFile(const QString &filename);

    /** returns true if there is a correct wav file to be streamed */
    bool isOK() const
    {
        return m_isOK;
    }

    /** Fill a float buffer with L/R stereo samples
        @param stereo_buffer a pointer to a float buffer
        @param stereo_samples number of stereo samples to write to buffer
        note: total bytes written to the buffer is sizeof(float)*2*stereo_samples.
        if no file was opened previously, the buffer will be filled with zeroes.
    */

    void fillBuffer(float *stereoBuffer, uint32_t stereoSamples);

    /** returns the filename of the currently loaded file. */
    QString GetFilename()
    {
        //QMutexLocker lock(&m_mutex);
        return m_filename;
    }

    /** fills output with format info
        @param output a reference to a WavFormatChunk to be filled
        @return true if operation was successful
    */
    bool getFormat(WavFormatChunk &output) const;

protected:
    //QMutex  m_mutex;            // mutex for protecting loading & buffer filling.

    bool findChunk(const char ID[4]);

    /** ReadSamples fills temp_buffer with data.
        Actual byte-count depends on the sample size (16 bit, 24 bit, 32 bit or float)
    */
    uint32_t readRawData(uint32_t requestedSamples);

    QFile       m_file;
    QDataStream *m_waveStream;          // the file to be used
    QString     m_filename;

    uint32_t    m_riffSize;
    int32_t     m_playOffset;      // playback offset (into file)
    int32_t     m_playStart;       // start of audio data within .wav file
    int32_t     m_playEnd;         // end of audio data within .wav file

    char        m_chunkType[4];         // type of chunk
    uint32_t    m_chunkSize;            // size of chunk in bytes

    WavFormatChunk  m_waveFormat;

    uint32_t    sampleIndex;           // where to start reading the temp_buffer (in mono sample offset) 0..8191
    void        *tempBuffer;           // holds temporary file data, has 4096 stereo sample entries.
    bool        m_isOK;
};

#endif  //Sentry
