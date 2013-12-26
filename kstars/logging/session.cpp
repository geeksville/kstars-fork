/***************************************************************************
                    session.cpp - K Desktop Planetarium
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

#include "session.h"

using namespace Logging;

Session::Session(const int id, const QDateTime &begin, const QDateTime &end, const int siteId,
                 const QList<int> &observersIds, const QString &weatherDesc,
                 const QString &equipmentDesc, const QString &comments, const QString &language,
                 const QStringList &images) :
    m_Id(id), m_Begin(begin), m_End(end), m_SiteId(siteId), m_ObserversIds(observersIds),
    m_WeatherDescription(weatherDesc), m_EquipmentDescription(equipmentDesc),
    m_Comments(comments), m_Language(language), m_Images(images)
{ }

