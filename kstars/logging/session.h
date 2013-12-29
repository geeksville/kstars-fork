/***************************************************************************
                    session.h - K Desktop Planetarium
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

#ifndef SESSION_H
#define SESSION_H

#include "QDateTime"
#include "QStringList"

#include "optional.h"

namespace Logging
{

class Session
{
public:
    Session(const int id, const QDateTime &begin, const QDateTime &end, const int siteId);

    Session(const int id, const QDateTime &begin, const QDateTime &end, const int siteId,
            const QList<int> &observersIds, const QString &weatherDesc,
            const QString &equipmentDesc, const QString &comments, const QString &language,
            const QStringList &images);

    int id() const
    {
        return m_Id;
    }

    QDateTime begin() const
    {
        return m_Begin;
    }

    QDateTime end() const
    {
        return m_End;
    }

    int siteId() const
    {
        return m_SiteId;
    }

    QList<int> observersIds() const
    {
        return m_ObserversIds;
    }

    QString weatherDesc() const
    {
        return m_WeatherDescription;
    }

    QString equipmentDesc() const
    {
        return m_EquipmentDescription;
    }

    QString comments() const
    {
        return m_Comments;
    }

    QString language() const
    {
        return m_Language;
    }

    QStringList images() const
    {
        return m_Images;
    }

    void setBegin(const QDateTime &begin)
    {
        m_Begin = begin;
    }

    void setEnd(const QDateTime &end)
    {
        m_End = end;
    }

    void setSiteId(const int siteId)
    {
        m_SiteId = siteId;
    }

    void setObserversIds(const QList<int> observersIds)
    {
        m_ObserversIds = observersIds;
    }

    void setWeatherDescription(const QString &weatherDescription)
    {
        m_WeatherDescription = weatherDescription;
    }

    void setEquipmentDescription(const QString &equipmentDescription)
    {
        m_EquipmentDescription = equipmentDescription;
    }

    void setComments(const QString &comments)
    {
        m_Comments = comments;
    }

    void setLanguage(const QString &language)
    {
        m_Language = language;
    }

    void setImages(const QStringList &images)
    {
        m_Images = images;
    }

private:
    int m_Id;
    QDateTime m_Begin;
    QDateTime m_End;
    int m_SiteId;
    QList<int> m_ObserversIds;
    QString m_WeatherDescription;
    QString m_EquipmentDescription;
    QString m_Comments;
    QString m_Language;
    QStringList m_Images;
};

}

#endif // SESSION_H
