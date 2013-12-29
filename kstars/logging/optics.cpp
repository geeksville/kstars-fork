/***************************************************************************
                    optics.cpp - K Desktop Planetarium
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

#include "optics.h"

using namespace Logging;

Optics::Optics(const int id, const QString &model, const double aperture) :
    m_Id(id), m_Model(model), m_Aperture(aperture)
{ }

Optics::Optics(const int id, const QString &model, const QString &type, const QString &vendor,
               const double aperture, const double lightGrasp, const bool orientationErect,
               const bool orientationTruesided) :
    m_Id(id), m_Model(model), m_Type(type), m_Vendor(vendor), m_Aperture(aperture), m_LightGrasp(lightGrasp),
    m_OrientationErect(orientationErect), m_OrientationTruesided(orientationTruesided)
{ }
