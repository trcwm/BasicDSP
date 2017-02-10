 #-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T16:49:53
#
#-------------------------------------------------

CONFIG   += c++11
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BasicDSP
TEMPLATE = app


################################################################################
# Portaudio stuff
################################################################################

INCLUDEPATH += contrib/portaudio/include \
               contrib/portaudio/src/common

SOURCES +=  contrib/portaudio/src/common/pa_allocation.c \
            contrib/portaudio/src/common/pa_converters.c \
            contrib/portaudio/src/common/pa_cpuload.c \
            contrib/portaudio/src/common/pa_debugprint.c \
            contrib/portaudio/src/common/pa_dither.c \
            contrib/portaudio/src/common/pa_front.c \
            contrib/portaudio/src/common/pa_process.c \
            contrib/portaudio/src/common/pa_ringbuffer.c \
            contrib/portaudio/src/common/pa_stream.c \
            contrib/portaudio/src/common/pa_trace.c

## Windows specific stuff
win32 {
    DEFINES += PA_USE_WMME
    DEFINES += PA_USE_WASAPI
    DEFINES += PA_USE_DS
    DEFINES += PA_USE_WDMKS PA_WDMKS_NO_KSGUID_LIB
    DEFINES += PAWIN_USE_DIRECTSOUNDFULLDUPLEXCREATE
    DEFINES += _CRT_SECURE_NO_WARNINGS

    INCLUDEPATH += contrib/portaudio/src/os/win

    SOURCES += contrib/portaudio/src/os/win/pa_win_hostapis.c \
               contrib/portaudio/src/os/win/pa_win_util.c \
               contrib/portaudio/src/os/win/pa_win_waveformat.c \
               contrib/portaudio/src/os/win/pa_x86_plain_converters.c \
               contrib/portaudio/src/os/win/pa_win_coinitialize.c

    SOURCES += contrib/portaudio/src/hostapi/dsound/pa_win_ds.c \
               contrib/portaudio/src/hostapi/dsound/pa_win_ds_dynlink.c \
               contrib/portaudio/src/hostapi/wdmks/pa_win_wdmks.c \
               contrib/portaudio/src/hostapi/wasapi/pa_win_wasapi.c \
               contrib/portaudio/src/hostapi/wmme/pa_win_wmme.c

    LIBS += winmm.lib dsound.lib user32.lib Advapi32.lib
}

## linux make
## needs: libasound2-dev
unix:!macx {
    DEFINES += PA_USE_ALSA

    INCLUDEPATH += contrib/portaudio/src/os/unix

    SOURCES += contrib/portaudio/src/os/unix/pa_unix_hostapis.c \
               contrib/portaudio/src/os/unix/pa_unix_util.c

    SOURCES += contrib/portaudio/src/hostapi/alsa/pa_linux_alsa.c

    LIBS += -lasound
}

## OSX
macx {
    DEFINES -= __linux__
    DEFINES += PA_USE_COREAUDIO

    INCLUDEPATH += contrib/portaudio/src/hostapi/coreaudio
    INCLUDEPATH += contrib/portaudio/src/os/unix

    SOURCES += contrib/portaudio/src/hostapi/coreaudio/pa_mac_core.c \
               contrib/portaudio/src/hostapi/coreaudio/pa_mac_core_blocking.c \
               contrib/portaudio/src/hostapi/coreaudio/pa_mac_core_utilities.c \
               contrib/portaudio/src/os/unix/pa_unix_util.c \
               contrib/portaudio/src/os/unix/pa_unix_hostapis.c

    LIBS += -framework CoreAudio -framework CoreServices
    LIBS += -framework AudioUnit -framework AudioToolbox
}

################################################################################
# Kiss FFT stuff
################################################################################

SOURCES += contrib/kiss_fft130/kiss_fft.c
HEADERS += contrib/kiss_fft130/kiss_fft.h
INCLUDEPATH += contrib/kiss_fft130

################################################################################
# Main Basic DSP sources
################################################################################

RC_FILE = resources/basicdsp.rc

SOURCES += main.cpp\
        vumeter.cpp\
        virtualmachine.cpp\
        namedslider.cpp\
        parser.cpp\
        tokenizer.cpp\
        reader.cpp\
        logging.cpp\
        asttovm.cpp\
        fft.cpp\
        portaudio_helper.cpp\
        spectrumwidget.cpp\
        spectrumwindow.cpp\
        scopewindow.cpp\
        scopewidget.cpp\
        codeeditor.cpp\
        mainwindow.cpp\
        functiondefs.cpp\
        soundcarddialog.cpp\
        aboutdialog.cpp\
        wavstreamer.cpp


HEADERS  += mainwindow.h\
            vumeter.h\
            namedslider.h\
            parser.h\
            tokenizer.h\
            reader.h\
            logging.h\
            asttovm.h\
            fft.h\
            portaudio_helper.h\
            spectrumwidget.h\
            spectrumwindow.h\
            virtualmachine.h \
            scopewindow.h\
            scopewidget.h \
            codeeditor.h\
            functiondefs.h\
            soundcarddialog.h\
            aboutdialog.h\
            wavstreamer.h

FORMS    += mainwindow.ui \
    spectrumwindow.ui \
    scopewindow.ui \
    soundcarddialog.ui \
    aboutdialog.ui

RESOURCES += \
    resources.qrc
