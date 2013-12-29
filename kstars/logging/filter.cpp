/***************************************************************************
                    filter.cpp - K Desktop Planetarium
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

#include "filter.h"

using namespace Logging;

Filter::Filter(const int id, const QString &model, const QString &type) :
    m_Id(id), m_Model(model), m_Type(type)
{ }

Filter::Filter(const int id, const QString &model, const QString &vendor, const QString &type,
               const QString &color, const QString &wratten, const QString &schott) :
    m_Id(id), m_Model(model), m_Vendor(vendor), m_Type(type), m_Color(color),
    m_Wratten(wratten), m_Schott(schott)
{ }
