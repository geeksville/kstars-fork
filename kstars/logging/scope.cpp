/***************************************************************************
                    scope.cpp - K Desktop Planetarium
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

#include "scope.h"

using namespace Logging;

Scope::Scope(const int id, const QString &model, const double aperture, const double focalLength) :
    Optics(id, model, aperture), m_FocalLength(focalLength)
{ }

Scope::Scope(const int id, const QString &model, const QString &type, const QString &vendor,
             const double aperture, const double lightGrasp, const bool orientationErect,
             const bool orientationTruesided, const double focalLength) :
    Optics(id, model, type, vendor, aperture, lightGrasp, orientationErect, orientationTruesided),
    m_FocalLength(focalLength)
{ }
