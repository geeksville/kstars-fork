/***************************************************************************
                    site.h - K Desktop Planetarium
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

#ifndef SITE_H
#define SITE_H

#include "optional.h"
#include "dms.h"

namespace Logging
{

class Site
{
public:
    Site(const int id, const QString &name, const dms &longitude, const dms &latitude);

    Site(const int id, const QString &name, const dms &longitude, const dms &latitude,
         const double elevation, const int timezone, const int code);

    int id() const
    {
        return m_Id;
    }

    QString name() const
    {
        return m_Name;
    }

    dms longitude() const
    {
        return m_Longitude;
    }

    dms latitude() const
    {
        return m_Latitude;
    }

    Optional<double> elevation() const
    {
        return m_Elevation;
    }

    Optional<int> timezone() const
    {
        return m_Timezone;
    }

    Optional<int> IAUCode() const
    {
        return m_IAUCode;
    }

    void setName(const QString &name)
    {
        m_Name = name;
    }

    void setLongitude(const dms &longitude)
    {
        m_Longitude = longitude;
    }

    void setLatitude(const dms &latitude)
    {
        m_Latitude = latitude;
    }

    void setElevation(const Optional<double> &elevation)
    {
        m_Elevation = elevation;
    }

    void setTimezone(const Optional<int> &timezone)
    {
        m_Timezone = timezone;
    }

    void setIAUCode(const Optional<int> &code)
    {
        m_IAUCode = code;
    }

private:
    int m_Id;
    QString m_Name;
    dms m_Longitude;
    dms m_Latitude;
    Optional<double> m_Elevation;
    Optional<int> m_Timezone;
    Optional<int> m_IAUCode;
};

}

#endif // SITE_H
