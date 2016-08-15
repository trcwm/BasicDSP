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

#include "portaudio_helper.h"

QString PA_Helper::getDeviceString(PaDeviceIndex index)
{
    const PaDeviceInfo *info = Pa_GetDeviceInfo(index);
    if (info == NULL)
        return QString("Device not found");

    QString deviceName = QString(info->name);

    const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(info->hostApi);
    if (apiInfo != 0)
    {
        deviceName.append(" [");
        deviceName.append(apiInfo->name);
        deviceName.append("|");
        deviceName.append(QString::number(info->maxInputChannels));
        deviceName.append("|");
        deviceName.append(QString::number(info->maxOutputChannels));
        deviceName.append("]");
    }
    else
    {
        deviceName.append(" [hostapi error]");
    }
    return deviceName;
}

PaDeviceIndex PA_Helper::getDeviceIndexByName(const QString &name)
{
    PaDeviceIndex count = Pa_GetDeviceCount();
    for(PaDeviceIndex i=0; i<count; i++)
    {
        if (getDeviceString(i)==name)
            return i;
    }
    return paNoDevice;  // no device
}
