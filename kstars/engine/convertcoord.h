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

    // FIXME: What's the best way to represent the time + place data for
    // this conversion, anyways?
    // I sort of feel that it may be best to give a geolocation and a time,
    // and then have it figure out the LST internally, but I'm not 100% sure.

    /** @return rotation from horizontal to equatorial coordinates
     *  @param LST the local sidereal time
     *  @param lat the latitude of the observer
     */
    CoordConversion HorToEq( const dms &LST,
                             const dms &lat );

    /** @return rotation from equatorial to horizontal coordinates
     *  @param LST the local sidereal time
     *  @param lat the latitude of the observer
     */
    CoordConversion EqToHor( const dms &LST,
                             const dms &lat );

    /** @return rotation from ecliptic to equatorial coordinates
     *  @param jd the date
     */
    CoordConversion EclToEq( const JulianDate jd );

    /** @return rotation from equatorial to ecliptic coordinates 
     *  @param jd the date
     */
    CoordConversion EqToEcl( const JulianDate jd );

    /** @return rotation from equatorial coordinates to J2000 coordinates
     *  @param jd the date the equatorial coordinates are defined for
     */
    CoordConversion EqToJ2000( const JulianDate jd );

    /** @return rotation from J2000 coordinates to equatorial coordinates
     *  @param jd the date the equatorial coordinates are defined for
     */
    CoordConversion J2000ToEq( const JulianDate jd );

    /** @return rotation representing precession from J2000.
     *  @param jd the date to precess to
     */
    CoordConversion PrecessTo( const JulianDate jd );

    /** @return rotation representing deprecession to J2000.
     *  @param jd the date to precess from.
     */
    CoordConversion PrecessFrom( const JulianDate jd );

    /** @return a rotation representing the nutation for this date.
     *  @param jd the date in question.
     */
    CoordConversion Nutate( const JulianDate jd );

    /** @return a rotation which removes nutation.
     *  @param jd the date
     */
    CoordConversion DeNutate( const JulianDate jd );

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

