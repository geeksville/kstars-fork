/***************************************************************************
                    lens.h - K Desktop Planetarium
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

#ifndef LENS_H
#define LENS_H

#include "QString"

namespace Logging
{

class Lens
{
public:
    Lens(const int id, const QString &model, const QString &vendor, const double factor);

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

    double factor() const
    {
        return m_Factor;
    }

    void setModel(const QString &model)
    {
        m_Model = model;
    }

    void setVendor(const QString &vendor)
    {
        m_Vendor = vendor;
    }

    void setFactor(const double factor)
    {
        m_Factor = factor;
    }

private:
    int m_Id;
    QString m_Model;
    QString m_Vendor;
    double m_Factor;
};

}

#endif // LENS_H
