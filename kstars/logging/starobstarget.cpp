/***************************************************************************
                    starobstarget.cpp - K Desktop Planetarium
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

#include "starobstarget.h"

using namespace Logging;

StarObsTarget::StarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                             const int discovererObserverId, const dms &ra, const dms &dec,
                             const QString &constellation, const QString &notes, const double apparentMag,
                             const QString &classification) :
    ObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes),
    m_ApparentMag(apparentMag), m_Classification(classification)
{ }


