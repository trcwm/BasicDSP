#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T16:49:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BasicDSP
TEMPLATE = app

# Portaudio stuff
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
            contrib/portaudio/src/common/pa_trace.c \
    scopewindow.cpp
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


# Main Basic DSP sources

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
        spectrumwidget.cpp\
        spectrumwindow.cpp\
        scopewindow.cpp\
        scopewidget.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h\
            vumeter.h\
            namedslider.h\
            parser.h\
            tokenizer.h\
            reader.h\
            logging.h\
            asttovm.h\
            spectrumwidget.h\
            spectrumwindow.h\
            virtualmachine.h \
            scopewindow.h\
            scopewidget.h

FORMS    += mainwindow.ui \
    spectrumwindow.ui \
    scopewindow.ui
