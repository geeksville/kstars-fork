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

#include <Eigen/Geometry>

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
 * We represent all of these as a Quaternionf, which gives the rotation from
 * the origin of the coordinate system to the given point.
 * Changing coordinate systems is then as simple as computing the rotation of
 * one coordinate system's origin to the next, and multiplying (28 flop).
 *  
 * FIXME: Should it be a compile error to convert one to the other without
 * using an explicit function? 
 * 
 ***************************************************************************/

typedef Quaternionf EquatorialCoord;
typedef Quaternionf HorizontalCoord;
typedef Quaternionf EclipticCoord;
typedef Quaternionf GalacticCoord;
typedef Quaternionf J2000Coord;
typedef Quaternionf B1950Coord;

/***************************************************************************
 * TIME
 ***************************************************************************/

typedef double JulianDate;

/// JulianDate for noon on Jan 1, 2000 (epoch J2000)
static const JulianDate EpochJ2000 = 2451545.0;

/// JulianDate for Jan 0.9235, 1950
static const JulianDate EpochB1950 = 2433282.4235;

/// Number of sidereal seconds in one solar second
static const double SiderealSecond = 1.002737909;

/***************************************************************************
 * LOCATION
 ***************************************************************************/

/// Rotation from the prime meridian.
typedef Quaternionf GreenwichCoord;

/// Rotation from lat = long = 0 to the observer's latitude and
/// local sidereal time.
typedef Quaternionf LocalSiderealCoords;
  
} // NS KSEngine

#endif //KSENGINE_TYPES_H