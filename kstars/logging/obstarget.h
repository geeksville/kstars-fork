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

#ifndef OBSERVATIONTARGET_H
#define OBSERVATIONTARGET_H

#include "dms.h"
#include "QStringList"

namespace Logging
{

class ObsTarget
{
public:
    ObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
              const int discovererObserverId, const dms &ra, const dms &dec,
              const QString &constellation, const QString &notes);

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

    dms ra() const
    {
        return m_Ra;
    }

    dms dec() const
    {
        return m_Dec;
    }

    // reference frame type

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

    void setRa(const dms &ra)
    {
        m_Ra = ra;
    }

    void setDec(const dms &dec)
    {
        m_Dec = dec;
    }

    // set reference frame type

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
    dms m_Ra;
    dms m_Dec;
    QString m_ReferenceFrameType;
    QString m_Constellation;
    QString m_Notes;
};

}

#endif // OBSERVATIONTARGET_H
