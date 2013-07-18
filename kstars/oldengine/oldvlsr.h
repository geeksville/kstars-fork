/***************************************************************************
     engine/oldvlsr.h - deprecated functions for the vlsr calculator.
                             -------------------
    begin                : 2013-06-11
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


#ifndef KSENGINE_OLDVLSR_H
#define KSENGINE_OLDVLSR_H

#include "ksengine/kstypes.h"

class SkyPoint;

namespace KSEngine {
/**
 * This namespace contains unported versions of old code
 * that used to be in the Skypoint class. These functions were only ever used
 * by the vlsr calculator, so they're seperate from the other old skypoint 
 * functions.
 * Using these is very inefficient, so existing code should, over time,
 * be ported to use new APIs.
 */
namespace OldVLSR {
    /** Computes the velocity of the Sun projected on the direction of the 
     *  source.
     * 
     *  @param s the source point
     *  @param jd Epoch to which the source coordinates refer.
     *  @return Radial velocity of the source referred to the barycenter of 
     *          the solar system in km/s
     **/
    double vRSun( const SkyPoint   &s,
                  const JulianDate  jd );

    /** Computes the radial velocity of a source referred to the solar system 
     *  barycenter from the radial velocity referred to the Local Standard of 
     *  Rest, aka VLSR. To compute it we need the coordinates of the
     *  source, the VLSR and the epoch for the source coordinates.
     *
     *  @param s the source point
     *  @param vlsr radial velocity of the source referred to the LSR in km/s
     *  @param jd Epoch to which the source coordinates refer.
     *  @return Radial velocity of the source referred to the barycenter of 
     *          the solar system in km/s
     **/
    double vHeliocentric( const SkyPoint   &s,
                          const double      vlsr,
                          const JulianDate  jd );

    /** Computes the radial velocity of a source referred to the Local 
     *  Standard of Rest, also known as VLSR, from the radial velocity 
     *  referred to the solar system barycenter.
     *
     *  @param s the source point
     *  @param vhelio radial velocity of the source referred to the LSR in km/s
     *  @param jd Epoch to which the source coordinates refer.
     *
     *  @return Radial velocity of the source referred to the barycenter of 
     *          the solar system in km/s
     **/
    double vHelioToVlsr( const SkyPoint   &s,
                         const double      vhelio,
                         const JulianDate  jd );

    /** Computes the velocity of any object projected on the direction 
     *  of the source.
     *
     *  @param s the source point
     *  @param jd0 Julian day for which we compute the direction of the source
     *
     *  @return velocity of the Earth projected on the direction of the source kms-1
     */
    double vREarth( const SkyPoint   &s,
                    const JulianDate  jd );

    /** Computes the radial velocity of a source referred to the center 
     *  of the earth from the radial velocity referred to the solar system 
     *  barycenter.
     *
     *  @param s the source point
     *  @param vhelio radial velocity of the source referred to the barycenter 
     *         of the solar system in km/s
     *  @param jd Epoch to which the source coordinates refer.
     *
     *  @return Radial velocity of the source referred to the 
     *          center of the Earth in km/s
     **/
    double vGeocentric( const SkyPoint   &s,
                        const double      vhelio,
                        const JulianDate  jd );

    /** Computes the radial velocity of a source referred to the solar
     *  system barycenter from the velocity referred to the center of the earth
     *
     *  @param s the source point
     *  @param vgeo radial velocity of the source referred to the 
     *         center of the Earth [km/s]
     *  @param jd Epoch which the source coordinates refer to.
     *
     *  @return Radial velocity of the source referred to the solar 
     *          system barycenter in km/s
     **/
    double vGeoToVHelio( const SkyPoint   &s,
                         const double      vgeo,
                         const JulianDate  jd);

    /** Computes the velocity of any object (oberver's site) projected on the
     *  direction of the source.
     *
     *  @param s the source point
     *  @param vsite velocity of that object in cartesian coordinates
     *
     *  @return velocity of the object projected on the direction of 
     *          the source kms-1
     */
    double vRSite( const SkyPoint &s, 
                   const double    vsite[3] );

    /** Computes the radial velocity of a source referred to the observer
     *  site on the surface of the earth from the geocentric velocity and
     *  the velocity of the site referred to the center of the Earth.
     *
     *  @param s the source point
     *  @param vgeo radial velocity of the source referred to the center
     *         of the earth in km/s
     *  @param vsite Velocity at which the observer moves referred to
     *         the center of the earth.
     *
     *  @return Radial velocity of the source referred to the observer's 
     *          site in km/s
     **/
    double vTopocentric( const SkyPoint &s,
                         const double    vgeo, 
                         const double    vsite[3] );

    /** Computes the radial velocity of a source referred to the center of 
     *  the Earth from the radial velocity referred to an observer site on 
     *  the surface of the earth.
     *
     *  @param s the source point
     *  @param vtopo radial velocity of the source referred to the 
     *         observer's site in km/s
     *  @param vsite Velocity at which the observer moves referred to the 
     *         center of the earth.
     *
     *  @return Radial velocity of the source referred the center of the 
     *          earth in km/s
     **/
    double vTopoToVGeo( const SkyPoint &s,
                        const double    vtopo,
                        const double    vsite[3]);

} //NS OldVLSR
} //NS KSEngine

#endif //KSENGINE_OLDVLSR_H