/*  INDI STD
    Copyright (C) 2012 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    Handle INDI Standard properties.
 */

#include "indistd.h"
#include "indicommon.h"
#include "clientmanager.h"
#include "driverinfo.h"


#include "skypoint.h"
#include "kstars.h"
#include "kstarsdata.h"
#include "skymap.h"
#include "Options.h"


#include <libindi/basedevice.h>

#include <QDebug>

#include <KMessageBox>
#include <KStatusBar>

const int MAX_FILENAME_LEN = 1024;

namespace ISD
{


GDSetCommand::GDSetCommand(INDI_TYPE inPropertyType, const QString &inProperty, const QString &inElement, QVariant qValue, QObject *parent) : QObject(parent)
{
    propType     = inPropertyType;
    indiProperty = inProperty;
    indiElement  = inElement;
    elementValue = qValue;
}

GenericDevice::GenericDevice(DriverInfo *idv)
{
    driverTimeUpdated = false;
    driverLocationUpdated = false;
    connected = false;

    driverInfo        = idv;

    Q_ASSERT(idv != NULL);

    baseDevice        = idv->getBaseDevice();
    clientManager     = idv->getClientManager();

    dType         = KSTARS_UNKNOWN;
}

GenericDevice::~GenericDevice()
{

}

const char * GenericDevice::getDeviceName()
{
    return baseDevice->getDeviceName();
}

void GenericDevice::registerProperty(INDI::Property *prop)
{

    foreach(INDI::Property *pp, properties)
    {
        if (pp == prop)
            return;
    }

    properties.append(prop);

    emit propertyDefined(prop);

    if (!strcmp(prop->getName(), "DEVICE_PORT"))
    {
        switch (driverInfo->getType())
        {
            case KSTARS_TELESCOPE:
            //qDebug() << "device port for Telescope!!!!!" << endl;
            if (Options::telescopePort().isEmpty() == false)
            {
                IText *tp = IUFindText(prop->getText(), "PORT");
                if (tp == NULL)
                    return;

                IUSaveText(tp, Options::telescopePort().toLatin1().constData());

                clientManager->sendNewText(prop->getText());

            }
            break;

        case KSTARS_CCD:
            //qDebug() << "device port for CCD!!!!!" << endl;
            if (Options::videoPort().isEmpty() == false)
            {
                IText *tp = IUFindText(prop->getText(), "PORT");
                if (tp == NULL)
                    return;

                IUSaveText(tp, Options::videoPort().toLatin1().constData());

                clientManager->sendNewText(prop->getText());

            }
            break;
            default:
                break;
        }

    }
}

void GenericDevice::removeProperty(INDI::Property *prop)
{
    properties.removeOne(prop);
}

void GenericDevice::processSwitch(ISwitchVectorProperty * svp)
{

    if (!strcmp(svp->name, "CONNECTION"))
    {
        ISwitch *conSP = IUFindSwitch(svp, "CONNECT");
        if (conSP == NULL)
            return;

        if (conSP->s == ISS_ON)
        {
            connected = true;
            emit Connected();
            createDeviceInit();
        }
        else
        {
            connected = false;
            emit Disconnected();
        }
    }


}

void GenericDevice::processNumber(INumberVectorProperty * nvp)
{
    INumber *np=NULL;

    if (!strcmp(nvp->name, "GEOGRAPHIC_LOCATION"))
    {
        if ( Options::useGeographicUpdate() && !driverLocationUpdated)
        {
            driverLocationUpdated = true;
            processDeviceInit();
        }

        // Update KStars Location once we receive update from INDI, if the source is set to DEVICE
        if (Options::useDeviceSource())
        {
            dms lng, lat;

            np = IUFindNumber(nvp, "LONG");
            if (!np)
                return;

        //sscanf(el->text.toAscii().data(), "%d%*[^0-9]%d%*[^0-9]%d", &d, &min, &sec);
            lng.setD(np->value);

           np = IUFindNumber(nvp, "LAT");

           if (!np)
                return;

        //sscanf(el->text.toAscii().data(), "%d%*[^0-9]%d%*[^0-9]%d", &d, &min, &sec);
        //lat.setD(d,min,sec);
         lat.setD(np->value);

        KStars::Instance()->data()->geo()->setLong(lng);
        KStars::Instance()->data()->geo()->setLat(lat);
      }

   }



}

void GenericDevice::processText(ITextVectorProperty * tvp)
{

    IText *tp=NULL;
    int d, m, y, min, sec, hour;
    QDate indiDate;
    QTime indiTime;
    KStarsDateTime indiDateTime;


    if (!strcmp(tvp->name, "TIME_UTC"))
    {
        if ( Options::useTimeUpdate() && driverTimeUpdated == false)
        {
            driverTimeUpdated = true;
            processDeviceInit();
        }

    // Update KStars time once we receive update from INDI, if the source is set to DEVICE
    if (Options::useDeviceSource())
    {
        tp = IUFindText(tvp, "UTC");

        if (!tp)
            return;

        sscanf(tp->text, "%d%*[^0-9]%d%*[^0-9]%dT%d%*[^0-9]%d%*[^0-9]%d", &y, &m, &d, &hour, &min, &sec);
        indiDate.setYMD(y, m, d);
        indiTime.setHMS(hour, min, sec);
        indiDateTime.setDate(indiDate);
        indiDateTime.setTime(indiTime);

        KStars::Instance()->data()->changeDateTime(indiDateTime);
        KStars::Instance()->data()->syncLST();
    }

    }
}

void GenericDevice::processLight(ILightVectorProperty * lvp)
{
    INDI_UNUSED(lvp);
}

void GenericDevice::processBLOB(IBLOB* bp)
{
    QFile       *data_file = NULL;
    INDIDataTypes dataType;

   if (!strcmp(bp->format, ".ascii"))
        dataType = DATA_ASCII;
    else
        dataType = DATA_OTHER;

    QString currentDir = Options::fitsDir();
    int nr, n=0;

    if (currentDir.endsWith('/'))
        currentDir.truncate(sizeof(currentDir)-1);

    QString filename(currentDir + '/');

    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");

    if (dataType == DATA_ASCII)
        filename += QString("file_") + ts + bp->format;
     else
        filename += QString("file_") + ts + '.' + bp->format;

    if (dataType == DATA_ASCII)
    {
        if (bp->aux0 == NULL)
        {
            bp->aux0 = new int();
            QFile * ascii_data_file = new QFile();
            ascii_data_file->setFileName(filename);
            if (!ascii_data_file->open(QIODevice::WriteOnly))
            {
                        kDebug() << "Error: Unable to open " << ascii_data_file->fileName() << endl;
                        return;
            }

            bp->aux1 = ascii_data_file;
        }

        data_file = (QFile *) bp->aux1;

        QDataStream out(data_file);
        for (nr=0; nr < (int) bp->size; nr += n)
           n = out.writeRawData( static_cast<char *> (bp->blob) + nr, bp->size - nr);

        out.writeRawData( (const char *) "\n" , 1);
        data_file->flush();
   }
    else
    {
        QFile fits_temp_file(filename);
        if (!fits_temp_file.open(QIODevice::WriteOnly))
        {
                kDebug() << "Error: Unable to open " << fits_temp_file.fileName() << endl;
        return;
        }

        QDataStream out(&fits_temp_file);

        for (nr=0; nr < (int) bp->size; nr += n)
            n = out.writeRawData( static_cast<char *> (bp->blob) +nr, bp->size - nr);

        fits_temp_file.close();
    }

    if (dataType == DATA_OTHER)
            KStars::Instance()->statusBar()->changeItem( i18n("Data file saved to %1", filename ), 0);

}

void GenericDevice::createDeviceInit()
{
    if ( Options::useTimeUpdate() && Options::useComputerSource())
    {
        ITextVectorProperty *tvp = baseDevice->getText("TIME_UTC");
        if (tvp && driverTimeUpdated ==false)
            updateTime();
    }

    if ( Options::useGeographicUpdate() && Options::useComputerSource())
    {
        INumberVectorProperty *nvp = baseDevice->getNumber("GEOGRAPHIC_COORD");
        if (nvp && driverLocationUpdated == false)
            updateLocation();
    }

    if ( Options::showINDIMessages() )
        KStars::Instance()->statusBar()->changeItem( i18n("%1 is online.", baseDevice->getDeviceName()), 0);

    KStars::Instance()->map()->forceUpdateNow();
}

void GenericDevice::processDeviceInit()
{

    if ( driverTimeUpdated && driverLocationUpdated && Options::showINDIMessages() )
        KStars::Instance()->statusBar()->changeItem( i18n("%1 is online and ready.", baseDevice->getDeviceName()), 0);
}


/*********************************************************************************/
/* Update the Driver's Time							 */
/*********************************************************************************/
void GenericDevice::updateTime()
{

    /* Update UTC */
    clientManager->sendNewNumber(baseDevice->getDeviceName(), "TIME_UTC_OFFSET", "OFFSET", KStars::Instance()->data()->geo()->TZ());

    QTime newTime( KStars::Instance()->data()->ut().time());
    QDate newDate( KStars::Instance()->data()->ut().date());

    QString isoTS = QString("%1-%2-%3T%4:%5:%6").arg(newDate.year()).arg(newDate.month()).arg(newDate.day()).arg(newTime.hour()).arg(newTime.minute()).arg(newTime.second());

    driverTimeUpdated = true;

    /* Update Date/Time */
    clientManager->sendNewText(baseDevice->getDeviceName(), "TIME_UTC", "UTC", isoTS.toLatin1().constData());

}

/*********************************************************************************/
/* Update the Driver's Geographical Location					 */
/*********************************************************************************/
void GenericDevice::updateLocation()
{
    GeoLocation *geo = KStars::Instance()->data()->geo();
    double longNP;

    if (geo->lng()->Degrees() >= 0)
        longNP = geo->lng()->Degrees();
    else
        longNP = dms(geo->lng()->Degrees() + 360.0).Degrees();

    INumberVectorProperty *nvp = baseDevice->getNumber("GEOGRAPHIC_COORD");

    if (nvp == NULL)
        return;

    INumber *np = IUFindNumber(nvp, "LONG");
    if (np == NULL)
        return;

    np->value = longNP;

    np = IUFindNumber(nvp, "LAT");
    if (np == NULL)
        return;

    np->value = geo->lat()->Degrees();

    driverLocationUpdated = true;

    clientManager->sendNewNumber(nvp);
}

bool GenericDevice::runCommand(int command, void *ptr)
{
    switch (command)
    {
       case INDI_CONNECT:
        clientManager->connectDevice(baseDevice->getDeviceName());
        break;

      case INDI_DISCONNECT:
        clientManager->disconnectDevice(baseDevice->getDeviceName());
        break;

       case INDI_SET_PORT:
       {
        if (ptr == NULL)
            return false;
        ITextVectorProperty *tvp = baseDevice->getText("DEVICE_PORT");
        if (tvp == NULL)
            return false;

        IText *tp = IUFindText(tvp, "PORT");

        IUSaveText(tp, (static_cast<QString *> (ptr))->toLatin1().constData() );

        clientManager->sendNewText(tvp);
       }
        break;

      // We do it here because it could be either CCD or FILTER interfaces, so no need to duplicate code
      case INDI_SET_FILTER:
      {
        if (ptr == NULL)
            return false;
        INumberVectorProperty *nvp = baseDevice->getNumber("FILTER_SLOT");
        if (nvp == NULL)
            return false;

        nvp->np[0].value = * ( (int *) ptr);

        clientManager->sendNewNumber(nvp);
      }

        break;

    }

    return true;
}

void GenericDevice::setProperty(QObject * setPropCommand)
{
    GDSetCommand *indiCommand = static_cast<GDSetCommand *> (setPropCommand);

    //qDebug() << "We are trying to set value for property " << indiCommand->indiProperty << " and element" << indiCommand->indiElement << " and value " << indiCommand->elementValue << endl;

    switch (indiCommand->propType)
    {
        case INDI_SWITCH:
        {
         INDI::Property *pp= baseDevice->getProperty(indiCommand->indiProperty.toLatin1().constData());
         if (pp == NULL)
             return;

        ISwitchVectorProperty *svp = pp->getSwitch();
        if (svp == NULL)
            return;

        ISwitch *sp = IUFindSwitch(svp, indiCommand->indiElement.toLatin1().constData());
        if (sp == NULL)
            return;

        if (svp->r == ISR_1OFMANY)
            IUResetSwitch(svp);

        sp->s = indiCommand->elementValue.toInt() == 0 ? ISS_OFF : ISS_ON;

        //qDebug() << "Sending switch " << sp->name << " with status " << ((sp->s == ISS_ON) ? "On" : "Off") << endl;
        clientManager->sendNewSwitch(svp);
       }
          break;

        // TODO: Add set property for other types of properties
    default:
        break;

    }
}

DeviceDecorator::DeviceDecorator(GDInterface *iPtr)
{
    interfacePtr = iPtr;

    baseDevice    = interfacePtr->getDriverInfo()->getBaseDevice();
    clientManager = interfacePtr->getDriverInfo()->getClientManager();
}

DeviceDecorator::~DeviceDecorator()
{
    delete (interfacePtr);
}

bool DeviceDecorator::runCommand(int command, void *ptr)
{
    return interfacePtr->runCommand(command, ptr);
}

void DeviceDecorator::setProperty(QObject *setPropCommand)
{
    interfacePtr->setProperty(setPropCommand);
}

void DeviceDecorator::processBLOB(IBLOB *bp)
{
    interfacePtr->processBLOB(bp);
    emit BLOBUpdated(bp);

}

void DeviceDecorator::processLight(ILightVectorProperty *lvp)
{
    interfacePtr->processLight(lvp);
    emit lightUpdated(lvp);

}

void DeviceDecorator::processNumber(INumberVectorProperty *nvp)
{
    interfacePtr->processNumber(nvp);
    emit numberUpdated(nvp);
}

void DeviceDecorator::processSwitch(ISwitchVectorProperty *svp)
{
    interfacePtr->processSwitch(svp);
    emit switchUpdated(svp);
}

void DeviceDecorator::processText(ITextVectorProperty *tvp)
{
    interfacePtr->processText(tvp);
    emit textUpdated(tvp);
}

void DeviceDecorator::registerProperty(INDI::Property *prop)
{
    interfacePtr->registerProperty(prop);
}

void DeviceDecorator::removeProperty(INDI::Property *prop)
{
    interfacePtr->removeProperty(prop);
    emit propertyDeleted(prop);
}


DeviceFamily DeviceDecorator::getType()
{
    return interfacePtr->getType();
}

DriverInfo * DeviceDecorator::getDriverInfo()
{
    return interfacePtr->getDriverInfo();
}

const char * DeviceDecorator::getDeviceName()
{
    return interfacePtr->getDeviceName();
}

INDI::BaseDevice* DeviceDecorator::getBaseDevice()
{
    return interfacePtr->getBaseDevice();
}

QList<INDI::Property *> DeviceDecorator::getProperties()
{
    return interfacePtr->getProperties();
}

bool DeviceDecorator::isConnected()
{
    return interfacePtr->isConnected();
}







}

#include "indistd.moc"
