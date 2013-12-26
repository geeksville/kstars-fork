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

#include "dms.h"

namespace Logging
{

class Site
{
public:
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

    double elevation() const
    {
        return m_Elevation;
    }

    int timezone() const
    {
        return m_Timezone;
    }

    int code() const
    {
        return m_Code;
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

    void setElevation(const double elevation)
    {
        m_Elevation = elevation;
    }

    void setTimezone(const int timezone)
    {
        m_Timezone = timezone;
    }

    void setCode(const int code)
    {
        m_Code = code;
    }

private:
    int m_Id;
    QString m_Name;
    dms m_Longitude;
    dms m_Latitude;
    double m_Elevation;
    int m_Timezone;
    int m_Code;
};

}

#endif // SITE_H
