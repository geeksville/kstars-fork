/***************************************************************************
                    obstarget.cpp - K Desktop Planetarium
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

#include "obstarget.h"

using namespace Logging;

ObsTarget::ObsTarget(const int id, const QString &name) :
    m_Id(id), m_Name(name)
{ }

ObsTarget::ObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                     const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                     const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes) :
    m_Id(id), m_Name(name), m_Aliases(aliases), m_Datasource(datasource),
    m_DiscovererObserverId(discovererObserverId), m_Ra(ra), m_Dec(dec), m_RefFrameOrigin(origin),
    m_RefFrameEquinox(equinox), m_Constellation(constellation), m_Notes(notes)
{ }
