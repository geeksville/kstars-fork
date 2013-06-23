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

Vector3f sphericalToVector(const Radian lat, const Radian lon)
{
    return Vector3f( cos(lat)*sin(lon),
                     sin(lat),
                     cos(lat)*cos(lon) );
}

Vector3f sphericalToVector(const dms &lat, const dms &lon)
{
    return sphericalToVector(lat.radians(),lon.radians());
}

void vectorToSpherical(const Vector3f &v, Radian *lat, Radian *lon)
{
    *lat = asin(v.y());
    *lon = atan2(v.x(), v.z());
}

void vectorToSpherical(const Vector3f &v, dms *lat, dms *lon)
{
    lat->setRadians(asin(v.y()));
    lon->setRadians(atan2(v.x(), v.z()));
}

}
}
