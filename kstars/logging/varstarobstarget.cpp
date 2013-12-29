/***************************************************************************
                    varstarobstarget.cpp - K Desktop Planetarium
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

#include "varstarobstarget.h"

using namespace Logging;

VarStarObsTarget::VarStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                   const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                                   const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                                   const double apparentMag, const QString &classification, const QString &type,
                                   const double maxApparentMag, const double period) :
    StarObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, origin, equinox, constellation,
                  notes, apparentMag, classification), m_Type(type), m_MaxApparentMag(maxApparentMag), m_Period(period)
{ }
