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
namespace Convert {

Vector3d sphToVect(const Radian lat, const Radian lon)
{
    return Vector3d( cos(lat)*sin(lon),
                     sin(lat),
                     cos(lat)*cos(lon) );
}

Vector3d sphToVect(const dms &lat, const dms &lon)
{
    return sphToVect(lat.radians(),lon.radians());
}

void vectToSph(const Vector3d &v, Radian *lat, Radian *lon)
{
    *lat = asin(v.y());
    *lon = atan2(v.x(), v.z());
}

void vectToSph(const Vector3d &v, dms *lat, dms *lon)
{
    lat->setRadians(asin(v.y()));
    lon->setRadians(atan2(v.x(), v.z()));
}

CoordConversion B1950ToGal()
{
    Quaterniond rot1(AngleAxisd(-282.25*DEG2RAD,Vector3d::UnitY()));
    Quaterniond rot2(AngleAxisd(  -62.6*DEG2RAD,Vector3d::UnitZ()));
    Quaterniond rot3(AngleAxisd(     33*DEG2RAD,Vector3d::UnitY()));
    return rot3*rot2*rot1;
}

CoordConversion GalToB1950()
{
    return B1950ToGal().conjugate();
}

CoordConversion EqToEcl(const JulianDate jd)
{
    //Julian Centuries since J2000.0
    double T = ( jd - EpochJ2000 ) / 36525.;
    //Obliquity of the Ecliptic
    double U = T/100.0;
    double dObliq = -4680.93*U - 1.55*U*U + 1999.25*U*U*U
                    - 51.38*U*U*U*U - 249.67*U*U*U*U*U
                    - 39.05*U*U*U*U*U*U + 7.12*U*U*U*U*U*U*U
                    + 27.87*U*U*U*U*U*U*U*U + 5.79*U*U*U*U*U*U*U*U*U
                    + 2.45*U*U*U*U*U*U*U*U*U*U;
    double obliq = 23.43929111 + dObliq/3600.0;
    return Quaterniond(AngleAxisd(-obliq*DEG2RAD,Vector3d::UnitZ()));
}

CoordConversion EclToEq(const JulianDate jd)
{
    return EqToEcl(jd).conjugate();
}

}
}
