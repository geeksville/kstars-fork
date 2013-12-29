/***************************************************************************
                    varstarobstarget.h - K Desktop Planetarium
                             -------------------
    begin                : Sun Dec 8 2013
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

#ifndef VARSTAROBSTARGET_H
#define VARSTAROBSTARGET_H

#include "starobstarget.h"

namespace Logging
{

class VarStarObsTarget : public StarObsTarget
{
public:
    VarStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                     const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                     const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                     const double apparentMag, const QString &classification, const QString &type,
                     const double maxApparentMag, const double period);

    QString type() const
    {
        return m_Type;
    }

    double maxApparentMagnitude() const
    {
        return m_MaxApparentMag;
    }

    double period() const
    {
        return m_Period;
    }

    void setType(const QString &type)
    {
        m_Type = type;
    }

    void setMaxApparentMagnitude(const double maxApparentMag)
    {
        m_MaxApparentMag = maxApparentMag;
    }

    void setPeriod(const double period)
    {
        m_Period = period;
    }

private:
    QString m_Type;
    double m_MaxApparentMag;
    double m_Period;
};

}

#endif // VARSTAROBSTARGET_H
