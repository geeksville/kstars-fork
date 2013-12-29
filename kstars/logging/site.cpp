/***************************************************************************
                    site.cpp - K Desktop Planetarium
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

#include "site.h"

using namespace Logging;

Site::Site(const int id, const QString &name, const dms &longitude, const dms &latitude) :
    m_Id(id), m_Name(name), m_Longitude(longitude), m_Latitude(latitude)
{ }

Site::Site(const int id, const QString &name, const dms &longitude, const dms &latitude,
           const double elevation, const int timezone, const int code) :
    m_Id(id), m_Name(name), m_Longitude(longitude), m_Latitude(latitude),
    m_Elevation(elevation), m_Timezone(timezone), m_IAUCode(code)
{ }

