/***************************************************************************
                    eyepiece.cpp - K Desktop Planetarium
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

#include "eyepiece.h"

using namespace Logging;

Eyepiece::Eyepiece(const int id, const QString &model, const QString &vendor, const double focalLength,
                   const double maxFocalLength, const dms &apparentFov) :
    m_Id(id), m_Model(model), m_Vendor(vendor), m_FocalLength(focalLength),
    m_MaxFocalLength(maxFocalLength), m_ApparentFov(apparentFov)
{ }
