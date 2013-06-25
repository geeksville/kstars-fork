/***************************************************************************
  engine/convertcoord.h - functions for converting between coordinate types
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

#ifndef KSENGINE_CONVERTCOORD_H
#define KSENGINE_CONVERTCOORD_H

#include "types.h"

class dms;
namespace KSEngine {
/**
 *  We have a bunch of different coordinate systems, and in most cases these
 *  functions can do all the work of converting them.
 *
 *  The possible conversions are:
 *
 *  HorizontalCoord
 *    ^
 *    | Needs time + place
 *    v
 *  EquatorialCoord <--------------------------> EclipticCoord
 *    ^                    Needs obliquity
 *    | Needs time
 *    v
 *  J2000Coord
 *    ^
 *    |
 *    v
 *  B1950Coord
 *    ^
 *    |
 *    v
 *  GalacticCoord
 *
 * @note the conversion from equatorial to J2000 coordinates only takes into
 *       account the precession and nutation of the earth. If the objects in
 *       question have other kinds of motion (e.g., proper motion), this must
 *       be corrected seperately.
 *
 */
namespace Convert {

    /** @return A vector corresponding to the given angular coordinates
     *  @param lat the latitude-like angle (altitude, declination, etc)
     *  @param lon the longitude-like angle (azimuth, RA, etc)
     *  @note You should convert the result to the appropriate named type.
     */
    Vector3d sphToVect( const dms &lat,
                        const dms &lon );
    Vector3d sphToVect( const Radian lat,
                        const Radian lon );

    /** Convert a vector to spherical coordinates.
     *  @param v the vector
     *  @param lat a pointer to store the computed latitude in
     *  @param long a pointer to store the computed longitude in
     */
    void vectToSph( const Vector3d &v,
                          dms      *lat,
                          dms      *lon );
    void vectToSph( const Vector3d &v,
                          Radian   *lat,
                          Radian   *lon );

    //
    // Calculate the rotation factors between coordinate systems.
    // To convert a coordinate from one type to another, get the rotation
    // using these functions and multiply, like so:
    //
    // GalacticCoord g = ...;
    // B1950Coord e = Convert::GalToB1950(c) * h;
    //

    /** @return rotation from horizontal to equatorial coordinates
     *  @param c the time/place
     */
    CoordConversion HorToEq( const LocalSiderealCoords &c );

    /** @return rotation from equatorial to horizontal coordinates
     *  @param c the time/place
     */
    CoordConversion EqToHor( const LocalSiderealCoords &c );

    /** @return rotation from ecliptic to equatorial coordinates
     *  @param obliquity the obliquity angle
     */
    CoordConversion EclToEq( const Radian obliquity );

    /** @return rotation from equatorial to ecliptic coordinates 
     *  @param obliquity the obliquity angle
     */
    CoordConversion EqToEcl( const Radian obliquity );

    /** @return rotation from equatorial coordinates to J2000 coordinates
     *  @param jd the date the equatorial coordinates are defined for
     */
    CoordConversion EqToJ2000( const JulianDate jd );

    /** @return rotation from J2000 coordinates to equatorial coordinates
     *  @param jd the date the equatorial coordinates are defined for
     */
    CoordConversion J2000ToEq( const JulianDate jd );

    /** @return rotation from J2000 coordinates to B1950 coordinates.
     */
    CoordConversion J2000ToB1950();

    /** @return rotation from B1950 coordinates to J2000 coordinates
     */
    CoordConversion B1950ToJ2000();

    /** @return rotation from B1950 coordinates to galactic coordinates
     */
    CoordConversion B1950ToGal();

    /** @return rotation from galactic coordinates to B1950 coordinates
     */
    CoordConversion GalToB1950();

}
}

#endif //KSENGINE_CONVERTCOORD_H

