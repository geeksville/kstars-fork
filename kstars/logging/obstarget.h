/***************************************************************************
                    obstarget.h - K Desktop Planetarium
                             -------------------
    begin                : Tue Nov 12 2013
    copyright            : (C) 2013 by Rafal Kulaga
    email                : rl.kulaga@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OBSTARGET_H
#define OBSTARGET_H

#include "optional.h"
#include "dms.h"
#include "QStringList"

namespace Logging
{

class ObsTarget
{
public:
    enum REF_FRAME_ORIGIN
    {
        RFO_GEOCENTRIC = 0,
        RFO_TOPOCENTRIC = 1
    };

    enum REF_FRAME_EQUINOX
    {
        RFT_J2000 = 0,
        RFT_B1950 = 1,
        RFT_EQ_OF_DATE = 2
    };

    ObsTarget(const int id, const QString &name);

    ObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
              const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
              const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes);

    QString name() const
    {
        return m_Name;
    }

    QStringList aliases() const
    {
        return m_Aliases;
    }

    QString datasource() const
    {
        return m_Datasource;
    }

    int discovererObserverId() const
    {
        return m_DiscovererObserverId;
    }

    Optional<dms> ra() const
    {
        return m_Ra;
    }

    Optional<dms> dec() const
    {
        return m_Dec;
    }

    Optional<REF_FRAME_ORIGIN> refFrameOrigin() const
    {
        return m_RefFrameOrigin;
    }

    Optional<REF_FRAME_EQUINOX> refFrameEquinox() const
    {
        return m_RefFrameEquinox;
    }

    QString constellation() const
    {
        return m_Constellation;
    }

    QString notes() const
    {
        return m_Notes;
    }

    void setName(const QString &name)
    {
        m_Name = name;
    }

    void setAliases(const QStringList &aliases)
    {
        m_Aliases = aliases;
    }

    void setDatasource(const QString &datasource)
    {
        m_Datasource = datasource;
    }

    void setDiscovererObserverId(const int observerId)
    {
        m_DiscovererObserverId = observerId;
    }

    void setRa(const Optional<dms> &ra)
    {
        m_Ra = ra;
    }

    void setDec(const Optional<dms> &dec)
    {
        m_Dec = dec;
    }

    void setRefFrameOrigin(const Optional<REF_FRAME_ORIGIN> &origin)
    {
        m_RefFrameOrigin = origin;
    }

    void setRefFrameEquinox(const Optional<REF_FRAME_EQUINOX> &equinox)
    {
        m_RefFrameEquinox = equinox;
    }

    void setConstellation(const QString &constellation)
    {
        m_Constellation = constellation;
    }

    void setNotes(const QString &notes)
    {
        m_Notes = notes;
    }

private:
    int m_Id;
    QString m_Name;
    QStringList m_Aliases;
    QString m_Datasource;
    int m_DiscovererObserverId;
    Optional<dms> m_Ra;
    Optional<dms> m_Dec;
    Optional<REF_FRAME_ORIGIN> m_RefFrameOrigin;
    Optional<REF_FRAME_EQUINOX> m_RefFrameEquinox;
    QString m_ReferenceFrameType;
    QString m_Constellation;
    QString m_Notes;
};

}

#endif // OBSTARGET_H
