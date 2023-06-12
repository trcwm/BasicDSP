##  BasicDSP Source Code
##
##  SPDX-License-Identifier: GPL-2.0-only
##  SPDX-FileCopyrightText: 2022 Niels Moseley <sofascener@gmail.com>
##  

cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

############################################################
## Setup BasicDSP version string
############################################################

project(basicdsp VERSION 2.1.0 HOMEPAGE_URL https://www.github.com/trcwm/basicdsp)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("!!!! Building debug binaries !!!!")
    set(BASICDSPVERSIONSTRING "BasicDSP version ${CMAKE_PROJECT_VERSION} DEBUG")
ELSE()
    set(BASICDSPVERSIONSTRING "BasicDSP version ${CMAKE_PROJECT_VERSION}")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

message(STATUS BasicDSP Version: ${BASICDSPVERSIONSTRING})

configure_file(${PROJECT_SOURCE_DIR}/src/version.cpp.in ${PROJECT_SOURCE_DIR}/src/version.cpp @ONLY)

############################################################
## Setup KissFFT
############################################################

add_library(kissfft 
    contrib/kiss_fft130/kiss_fft.c
    )

target_include_directories(kissfft INTERFACE contrib/kiss_fft130)

############################################################
## Setup PortAudio
############################################################

set (PA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/contrib/portaudio)

set (PORTAUDIO 
    ${PA_DIR}/src/common/pa_allocation.c
    ${PA_DIR}/src/common/pa_converters.c
    ${PA_DIR}/src/common/pa_cpuload.c
    ${PA_DIR}/src/common/pa_debugprint.c
    ${PA_DIR}/src/common/pa_dither.c
    ${PA_DIR}/src/common/pa_front.c
    ${PA_DIR}/src/common/pa_process.c
    ${PA_DIR}/src/common/pa_ringbuffer.c
    ${PA_DIR}/src/common/pa_stream.c
    ${PA_DIR}/src/common/pa_trace.c
)

if (UNIX)
    find_package(ALSA REQUIRED)
    #find_package(Jack REQUIRED)
    add_definitions(-DPA_USE_ALSA)
    #add_definitions(-DPA_USE_JACK)
    
    set (PORTAUDIO ${PORTAUDIO}
        ${PA_DIR}/src/os/unix/pa_unix_hostapis.c
        ${PA_DIR}/src/os/unix/pa_unix_util.c
        ${PA_DIR}/src/hostapi/alsa/pa_linux_alsa.c
        #${PA_DIR}/src/hostapi/jack/pa_jack.c
    )

    add_library(portaudio STATIC ${PORTAUDIO})
    #target_include_directories(portaudio PRIVATE ${ALSA_INCLUDE_DIRS} ${JACK_INCLUDE_DIR} ${PA_DIR}/include ${PA_DIR}/src/common ${PA_DIR}/src/os/unix)
    target_include_directories(portaudio PRIVATE ${ALSA_INCLUDE_DIRS} ${PA_DIR}/include ${PA_DIR}/src/common ${PA_DIR}/src/os/unix)
    target_include_directories(portaudio INTERFACE ${PA_DIR}/include ${PA_DIR}/src/common)
    #target_link_libraries(portaudio ${ALSA_LIBRARIES} ${JACK_LIBRARY})
    target_link_libraries(portaudio ${ALSA_LIBRARIES})
endif (UNIX)

if (WIN32)
    add_definitions(-DPA_USE_WMME)
    add_definitions(-DPA_USE_WASAPI)
    add_definitions(-DPA_USE_DS)
    add_definitions(-DPA_USE_WDMKS)
    add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)
    add_definitions(-DPAWIN_USE_DIRECTSOUNDFULLDUPLEXCREATE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    set (PORTAUDIO ${PORTAUDIO}
        ${PA_DIR}/src/os/win/pa_win_hostapis.c 
        ${PA_DIR}/src/os/win/pa_win_util.c 
        ${PA_DIR}/src/os/win/pa_win_waveformat.c 
        ${PA_DIR}/src/os/win/pa_x86_plain_converters.c 
        ${PA_DIR}/src/os/win/pa_win_coinitialize.c    
        ${PA_DIR}/src/hostapi/dsound/pa_win_ds.c 
        ${PA_DIR}/src/hostapi/dsound/pa_win_ds_dynlink.c 
        ${PA_DIR}/src/hostapi/wdmks/pa_win_wdmks.c 
        ${PA_DIR}/src/hostapi/wasapi/pa_win_wasapi.c 
        ${PA_DIR}/src/hostapi/wmme/pa_win_wmme.c            
    )

    add_library(portaudio STATIC ${PORTAUDIO})
    target_include_directories(portaudio PRIVATE ${PA_DIR}/include ${PA_DIR}/src/common ${PA_DIR}/src/os/win)
    target_include_directories(portaudio INTERFACE ${PA_DIR}/include ${PA_DIR}/src/common)
    target_link_libraries(portaudio winmm dsound user32 Advapi32)

endif (WIN32)

if(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)

endif()

############################################################
## BasicDSP source stuff
############################################################

find_package(Qt6 REQUIRED QUIET COMPONENTS Core Widgets Gui)

add_executable(basicdsp
    src/resources.qrc
    src/aboutdialog.cpp
    src/aboutdialog.ui
    src/asttovm.cpp
    src/codeeditor.cpp
    src/fft.cpp
    src/functiondefs.cpp
    src/logging.cpp
    src/namedslider.cpp
    src/parser.cpp
    src/portaudio_helper.cpp
    src/reader.cpp
    src/scopewidget.cpp
    src/scopewindow.cpp
    src/scopewindow.ui
    src/soundcarddialog.cpp
    src/soundcarddialog.ui
    src/spectrumwidget.cpp
    src/spectrumwindow.cpp
    src/spectrumwindow.ui
    src/tokenizer.cpp
    src/version.cpp
    src/virtualmachine.cpp
    src/vumeter.cpp
    src/wavstreamer.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    src/main.cpp
)

target_link_libraries(basicdsp Qt6::Widgets portaudio kissfft)

set_property(TARGET basicdsp PROPERTY AUTOMOC ON)
set_property(TARGET basicdsp PROPERTY AUTOUIC ON)
