/***************************************************************************
  engine/convertcoord.cpp - functions for converting between coordinate types
                             -------------------
    begin                : 2013-06-13
    copyright            : (C) 2013 by Henry de Valence
    email                : hdevalence@hdevalence.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "convertcoord.h"

#include "dms.h"

namespace KSEngine {
namespace ConvertCoord {

Quaternionf sphericalToQuaternion(const dms &lat, const dms &lon)
{
    // We rotate along the Z-axis first, and then the Y-axis.
    // Doing this in the opposite order is incorrect.
    // To convince yourself of this fact, spend some time playing with a
    // sperical object such as a grapefruit.
    Quaternionf lonq(AngleAxisf(lon.radians(), Vector3f::UnitZ()));
    Quaternionf latq(AngleAxisf(lat.radians(), Vector3f::UnitY()));
    return latq * lonq;
}

void quaternionToSpherical(const Quaternionf &q, dms *lat, dms *lon)
{
    const Vector3f origin(1,0,0);
    const Vector3f point = q * origin;
    lat->setRadians(asin(point.z()));
    lon->setRadians(atan2(point.y(), point.x()));
}

}
}