/***************************************************************************
  engine/oldconversions.h - deprecated functions for calculating precession.
                             -------------------
    begin                : 2013-06-12
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


#ifndef KSENGINE_OLDCONVERSIONS_H
#define KSENGINE_OLDCONVERSIONS_H

#include "ksengine/kstypes.h"

class SkyPoint;
class dms;

namespace KSEngine {
/**
 * This namespace contains unported versions of old coordinate conversion
 * code that used to be in the Skypoint class.
 * Using these is very inefficient, so existing code should, over time,
 * be ported to use new APIs.
 */
namespace OldConversions {
    /** Determine the (Altitude, Azimuth) coordinates of the
     *  SkyPoint from its (RA, Dec) coordinates, given the local
     *  sidereal time and the observer's latitude.
     *
     *  @param p the point in question
     *  @param LST pointer to the local sidereal time
     *  @param lat pointer to the geographic latitude
     */
    void EquatorialToHorizontal(       SkyPoint *p,
                                 const dms      *LST,
                                 const dms      *lat );

    /** Determine the (RA, Dec) coordinates of the
     *  SkyPoint from its (Altitude, Azimuth) coordinates, given the local
     *  sidereal time and the observer's latitude.
     *
     *  @param p the point in question
     *  @param LST pointer to the local sidereal time
     *  @param lat pointer to the geographic latitude
     */
    void HorizontalToEquatorial(       SkyPoint *p,
                                 const dms      *LST,
                                 const dms      *lat );

    /** Determine the Ecliptic coordinates of the SkyPoint, given the Julian Date.
     *  The ecliptic coordinates are returned as reference arguments (since
     *  they are not stored internally)
     *
     *  FIXME: document parameters properly.
     */
    void findEcliptic( const SkyPoint *p,
                       const dms      *Obliquity,
                             dms      &EcLong,
                             dms      &EcLat );

    /** Set the current (RA, Dec) coordinates of the
     *  SkyPoint, given pointers to its Ecliptic (Long, Lat) coordinates, and
     *  to the current obliquity angle (the angle between the equator and ecliptic).
     *
     *  FIXME: document parameters properly
     */
    void setFromEcliptic(       SkyPoint *p,
                          const dms      *Obliquity,
                          const dms      &EcLong,
                          const dms      &EcLat );

}
}

#endif //KSENGINE_OLDCONVERSIONS_H