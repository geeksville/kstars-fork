/***************************************************************************
                    optics.h - K Desktop Planetarium
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

#ifndef OPTICS_H
#define OPTICS_H

#include "QString"

namespace Logging
{

class Optics
{
public:
    Optics(const int id, const QString &model, const QString &type, const QString &vendor,
           const double aperture, const double lightGrasp, const bool orientationErect,
           const bool orientationTruesided);

    int id() const
    {
        return m_Id;
    }

    QString model() const
    {
        return m_Model;
    }

    QString type() const
    {
        return m_Type;
    }

    QString vendor() const
    {
        return m_Vendor;
    }

    double aperture() const
    {
        return m_Aperture;
    }

    double lightGrasp() const
    {
        return m_LightGrasp;
    }

    bool orientationErect() const
    {
        return m_OrientationErect;
    }

    bool orientationTruesided() const
    {
        return m_OrientationTruesided;
    }

    void setModel(const QString &model)
    {
        m_Model = model;
    }

    void setType(const QString &type)
    {
        m_Type = type;
    }

    void setVendor(const QString &vendor)
    {
        m_Vendor = vendor;
    }

    void setAperture(const double aperture)
    {
        m_Aperture = aperture;
    }

    void setLightGrasp(const double lightGrasp)
    {
        m_LightGrasp = lightGrasp;
    }

    void setOrientationTruesided(const bool isTruesided)
    {
        m_OrientationTruesided = isTruesided;
    }

    void setOrientationErect(const bool isErect)
    {
        m_OrientationErect = isErect;
    }


private:
    int m_Id;
    QString m_Model;
    QString m_Type;
    QString m_Vendor;
    double m_Aperture;
    double m_LightGrasp;
    bool m_OrientationErect;
    bool m_OrientationTruesided;
};

}

#endif // OPTICS_H
