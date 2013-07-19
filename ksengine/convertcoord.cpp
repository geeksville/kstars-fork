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

#include <Eigen/Geometry>

#include "astrovars.h"
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

Vector4d sphToVect4(const Radian lat, const Radian lon)
{
    return Vector4d( cos(lat)*sin(lon),
                     sin(lat),
                     cos(lat)*cos(lon),
                     0 );
}

Vector4d sphToVect4(const dms &lat, const dms &lon)
{
    return sphToVect4(lat.radians(),lon.radians());
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

void vectToSph(const Vector4d &v, Radian *lat, Radian *lon)
{
    *lat = asin(v.y());
    *lon = atan2(v.x(), v.z());
}

void vectToSph(const Vector4d &v, dms *lat, dms *lon)
{
    lat->setRadians(asin(v.y()));
    lon->setRadians(atan2(v.x(), v.z()));
}

CoordConversion B1950ToGal()
{
    Quaterniond rot1(AngleAxisd(-282.25*DEG2RAD,Vector3d::UnitY()));
    Quaterniond rot2(AngleAxisd(  -62.6*DEG2RAD,Vector3d::UnitZ()));
    Quaterniond rot3(AngleAxisd(     33*DEG2RAD,Vector3d::UnitY()));
    return (rot3*rot2*rot1).matrix();
}

CoordConversion GalToB1950()
{
    return B1950ToGal().transpose();
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
    return AngleAxisd(-obliq*DEG2RAD,Vector3d::UnitZ()).matrix();
}

CoordConversion EclToEq(const JulianDate jd)
{
    return EqToEcl(jd).transpose();
}

CoordConversion PrecessTo(const JulianDate jd)
{
    //Julian Centuries since J2000.0
    double T = ( jd - EpochJ2000 ) / 36525.;
    //Compute rotation angles from series
    double zeta  = 0.6406161*T + 0.0000839*T*T + 0.0000050*T*T*T;
    double theta = 0.5567530*T - 0.0001185*T*T - 0.0000116*T*T*T;
    double z     = 0.6406161*T + 0.0003041*T*T + 0.0000051*T*T*T;
    //Build rotation
    Quaterniond rot1(AngleAxisd(  zeta*DEG2RAD,Vector3d::UnitY()));
    Quaterniond rot2(AngleAxisd(-theta*DEG2RAD,Vector3d::UnitX()));
    Quaterniond rot3(AngleAxisd(     z*DEG2RAD,Vector3d::UnitY()));
    return (rot3*rot2*rot1).matrix();
}

CoordConversion PrecessFrom(const JulianDate jd)
{
    return PrecessTo(jd).transpose();
}

CoordConversion Nutate(const JulianDate jd)
{
    double dEcLong, dObliq;
    AstroVars::nutationVars(jd, &dEcLong, &dObliq);
    //Add dEcLong to the Ecliptic Longitude
    CoordConversion rot = AngleAxisd(dEcLong*DEG2RAD,Vector3d::UnitY()).matrix();
    return EclToEq(jd) * rot * EqToEcl(jd);
}

CoordConversion DeNutate(const JulianDate jd)
{
    return Nutate(jd).transpose();
}

EarthVelocityCoord Aberrate(const EarthVelocityCoord &p, const double expRapidity)
{
    if( p.y() < 0 ) {
        //Project through north pole.
        Vector2d p_proj = (1/(1-p.y()))*Vector2d(p.x(),p.z());
        Vector2d ab = expRapidity*p_proj;
        double n = ab.squaredNorm();
        // We use 0 and 1 to get the components we are calling X and Z
        return (1/(1+n))*Vector3d(2*ab(0), n-1, 2*ab(1));
    } else {
        //Project through south pole (same as projecting the point with
        //inverted y-coord through the north pole).
        Vector2d p_proj = (1/(1+p.y()))*Vector2d(p.x(),p.z());
        Vector2d ab = (1/expRapidity)*p_proj;
        double n = ab.squaredNorm();
        // We use 0 and 1 to get the components we are calling X and Z
        return (1/(1+n))*Vector3d(2*ab(0), -n+1, 2*ab(1));
    }
}

CoordConversion EqToHor(const dms &LST, const dms &lat)
{
    Quaterniond lstRot(AngleAxisd( -LST.radians(), Vector3d::UnitY()));
    Quaterniond latRot(AngleAxisd( -M_PI_2 + lat.radians(), Vector3d::UnitX()));
    Matrix3d flip;
    flip << 1, 0, 0,
            0, 1, 0,
            0, 0, -1;
    return flip * (latRot * lstRot).matrix();
}

CoordConversion HorToEq(const dms &LST, const dms &lat)
{
    return EqToHor(LST,lat).transpose();
}

CoordConversion EclToEarthVel(const JulianDate jd)
{
    Vector3d v = AstroVars::earthVelocity(jd);
    v.normalize();
    Quaterniond q = Quaterniond::FromTwoVectors(v,-Vector3d::UnitY());
    return q.matrix();
}

CoordConversion EarthVelToEcl(const JulianDate jd)
{
    return EclToEarthVel(jd).transpose();
}



}
}
