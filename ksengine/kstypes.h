/***************************************************************************
      engine/types.h  -  POD types for the stateless engine for KStars
                             -------------------
    begin                : 2013-06-10
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

#ifndef KSENGINE_TYPES_H
#define KSENGINE_TYPES_H

#include <Eigen/Core>

namespace KSEngine {

using namespace Eigen;

/***************************************************************************
 * COORDINATE SYSTEMS
 ***************************************************************************
 * 
 * We have a few main coordinate systems in use:
 * 
 *   * Equatorial coordinates.
 *   * Horizontal coordinates.
 *   * Ecliptic coordinates.
 *   * Galactic coordinates.
 *   * Catalog coordinates with respect to the J2000 epoch (equatorial).
 *   * Catalog coordinates with respect to the B1950 epoch (equatorial).
 * 
 * We represent all of these as a Vector3d, in the coordinate system
 *   
 *                 ^y
 *                 |
 *                 /----> x
 *                /
 *                z
 *
 * Changing coordinate systems is then as simple as computing the rotation of
 * one coordinate system to the next, and multiplying.
 *  
 * FIXME: Should it be a compile error to convert one to the other without
 * using an explicit function? 
 * 
 ***************************************************************************/

typedef Vector3d EquatorialCoord;
typedef Vector3d HorizontalCoord;
typedef Vector3d EclipticCoord;
typedef Vector3d GalacticCoord;
typedef Vector3d J2000Coord;
typedef Vector3d B1950Coord;

/*
 * These coordinate systems are used in conjunction with a
 * stereographic projection to compute some transformations without
 * making use of trigonometric functions. They are only defined up
 * to some rotation around the axis which is used for the projection,
 * but this is ok since the transformations have rotational symmetry.
 */

/// A coord system in which the Earth's velocity is aligned with -Y.
typedef Vector3d EarthVelocityCoord;

/// Represents a rotation from one coordinate system to another.
typedef Matrix3d CoordConversion;

/***************************************************************************
 * TIME
 ***************************************************************************/

/// Number of fractional days since beginning of Julian calendar.
typedef double JulianDate;

/// JulianDate for noon on Jan 1, 2000 (epoch J2000)
static const JulianDate EpochJ2000 = 2451545.0;

/// JulianDate for Jan 0.9235, 1950
static const JulianDate EpochB1950 = 2433282.4235;

/// Number of sidereal seconds in one solar second
static const double SiderealSecond = 1.002737909;

/***************************************************************************
 * ANGLES
 ***************************************************************************/

typedef double Radian;

/***************************************************************************
 * CONSTANTS
 ***************************************************************************/

/// Number of kilometers in one AU
static const double AU_KM = 1.49605e8;
/// The speed of light in km/sec.
static const double C_KMSEC = 299792.458;

/// Degrees to radians conversion
static const double DEG2RAD = M_PI/180.;
/// Arcmin to radians conversion
static const double ARCMIN2RAD = DEG2RAD/60.;
/// Arcsec to radions conversion
static const double ARCSEC2RAD = ARCMIN2RAD/60.;
/// Radians to degrees conversion
static const double RAD2DEG = 180./M_PI;
/// Number of radians in 1 milliarcsec
static const double MILLIARCSEC_RADIANS = 0.00000000484813681;
/// Number of degrees in 1 milliarcsec
static const double MILLIARCSEC_DEGREES = MILLIARCSEC_RADIANS*RAD2DEG;

} // NS KSEngine

#endif //KSENGINE_TYPES_H
