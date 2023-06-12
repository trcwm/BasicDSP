/*

  Description:  Portaudio-based device enumeration helper

  Portaudio does not (yet) pair input and output devices
  so we must try and figure out which device IDs can
  be used together.

  We do the following:
  * only pair device IDs that have the same host API ID.
  * only pair device IDs that support the same sample rates.

  As an option, we could open a full-duplex stream to
  verify if Portaudio will accept the paired device
  IDs.

  Author: Niels A. Moseley (c) 2016

*/

#ifndef portaudio_helper_h
#define portaudio_helper_h

#include <QString>
#include <portaudio.h>

namespace PA_Helper
{
    /** generate a unique device ID string */
    QString getDeviceString(PaDeviceIndex index);

    /** get the portaudio device index by device name */
    PaDeviceIndex getDeviceIndexByName(const QString &deviceName);
}

#endif
