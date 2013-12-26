/***************************************************************************
                    starobstarget.h - K Desktop Planetarium
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

#ifndef STAROBSERVATIONTARGET_H
#define STAROBSERVATIONTARGET_H

#include "obstarget.h"

namespace Logging
{

class StarObsTarget : public ObsTarget
{
public:
    StarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                  const int discovererObserverId, const dms &ra, const dms &dec,
                  const QString &constellation, const QString &notes, const double apparentMag,
                  const QString &classification);

    double apparentMag() const
    {
        return m_ApparentMag;
    }

    QString classification() const
    {
        return m_Classification;
    }

    void setApparentMag(const double mag)
    {
        m_ApparentMag = mag;
    }

    void setClassification(const QString &classification)
    {
        m_Classification = classification;
    }


private:
    double m_ApparentMag;
    QString m_Classification;
};

}

#endif // STAROBSERVATIONTARGET_H
