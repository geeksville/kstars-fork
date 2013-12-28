/***************************************************************************
                    obsfindings.h - K Desktop Planetarium
                             -------------------
    begin                : Mon Dec 2 2013
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

#ifndef OBSFINDINGS_H
#define OBSFINDINGS_H

#include "QString"

namespace Logging
{

class ObsFindings
{
public:
    ObsFindings(const int id, const QString &description, const QString &lang);

    int id() const
    {
        return m_Id;
    }

    QString description() const
    {
        return m_Description;
    }

    QString language() const
    {
        return m_Language;
    }

    void setDescription(const QString &description)
    {
        m_Description = description;
    }

    void setLanguage(const QString &lang)
    {
        m_Language = lang;
    }

private:
    int m_Id;
    QString m_Description;
    QString m_Language;
};

}

#endif // OBSFINDINGS_H
