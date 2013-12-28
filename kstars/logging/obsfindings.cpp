/***************************************************************************
                    obsfindings.cpp - K Desktop Planetarium
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

#include "obsfindings.h"

using namespace Logging;

ObsFindings::ObsFindings(const int id, const QString &description, const QString &lang) :
    m_Id(id), m_Description(description), m_Language(lang)
{ }
