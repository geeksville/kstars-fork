/***************************************************************************
                    filter.h - K Desktop Planetarium
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

#ifndef FILTER_H
#define FILTER_H

#include "QString"

namespace Logging
{

class Filter
{
public:
    Filter(const int id, const QString &model, const QString &type);

    Filter(const int id, const QString &model, const QString &vendor, const QString &type,
           const QString &color, const QString &wratten, const QString &schott);

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

    QString type() const
    {
        return m_Type;
    }

    QString color() const
    {
        return m_Color;
    }

    QString wratten() const
    {
        return m_Wratten;
    }

    QString schott() const
    {
        return m_Schott;
    }

    void setModel(const QString &model)
    {
        m_Model = model;
    }

    void setVendor(const QString &vendor)
    {
        m_Vendor = vendor;
    }

    void setType(const QString &type)
    {
        m_Type = type;
    }

    void setColor(const QString &color)
    {
        m_Color = color;
    }

    void setWratten(const QString &wratten)
    {
        m_Wratten = wratten;
    }

    void setSchott(const QString &schott)
    {
        m_Schott = schott;
    }


private:
    int m_Id;
    QString m_Model;
    QString m_Vendor;
    QString m_Type;
    QString m_Color;
    QString m_Wratten;
    QString m_Schott;
};

}

#endif // FILTER_H
