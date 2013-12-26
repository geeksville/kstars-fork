/***************************************************************************
                    eyepiece.h - K Desktop Planetarium
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

#ifndef EYEPIECE_H
#define EYEPIECE_H

#include "dms.h"
#include "QString"

namespace Logging
{

class Eyepiece
{
public:
    Eyepiece(const int id, const QString &model, const QString &vendor, const double focalLength,
             const double maxFocalLength, const dms &apparentFov);

    int id() const
    {
        return m_Id;
    }

    QString model() const
    {
        return m_Model;
    }

    QString vendor() const
    {
        return m_Vendor;
    }

    double focalLength() const
    {
        return m_FocalLength;
    }

    double maxFocalLength() const
    {
        return m_MaxFocalLength;
    }

    dms apparentFov() const
    {
        return m_ApparentFov;
    }

    void setModel(const QString &model)
    {
        m_Model = model;
    }

    void setVendor(const QString &vendor)
    {
        m_Vendor = vendor;
    }

    void setFocalLength(const double focalLength)
    {
        m_FocalLength = focalLength;
    }

    void setMaxFocalLength(const double maxFocalLength)
    {
        m_MaxFocalLength = maxFocalLength;
    }

    void setApparentFov(const dms &apparentFov)
    {
        m_ApparentFov = apparentFov;
    }


private:
    int m_Id;
    QString m_Model;
    QString m_Vendor;
    double m_FocalLength;
    double m_MaxFocalLength;
    dms m_ApparentFov;
};

}

#endif // EYEPIECE_H
