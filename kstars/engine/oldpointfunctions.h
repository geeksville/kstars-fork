/***************************************************************************
  engine/oldpointfunctions.h - deprecated functions for manipulating points.
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


#ifndef KSENGINE_OLDPOINTFUNCTIONS_H
#define KSENGINE_OLDPOINTFUNCTIONS_H

#include "types.h"

class KSSun;
class SkyPoint;
class KSPlanetBase;
class KSNumbers;
class dms;

namespace KSEngine {
/**
 * This namespace contains unported versions of old code
 * that used to be in the Skypoint class.
 * Using these is very inefficient, so existing code should, over time,
 * be ported to use new APIs.
 */
namespace OldPointFunctions {

/** Determine the current coordinates (RA, Dec) from the catalog
 *  coordinates (RA0, Dec0), accounting for both precession and nutation.
 *
 *  @param p pointer to the SkyPoint in question
 *  @param num pointer to KSNumbers object containing current values of
 *         time-dependent variables.
 *  @param forceRecompute reapplies precession, nutation and aberration even
 *         if the time passed since the last computation is not significant.
 */
void updateCoords(       SkyPoint  *p,
                   const KSNumbers *num,
                   const bool       forceRecompute=false );

/** Computes the apparent coordinates for this SkyPoint for any epoch,
 *  accounting for the effects of precession, nutation, and aberration.
 *  Similar to updateCoords(), but the starting epoch need not be
 *  J2000, and the target epoch need not be the present time.
 *
 *  @param p pointer to the SkyPoint in question
 *  @param jd0 Julian Day which identifies the original epoch
 *  @param jdf Julian Day which identifies the final epoch
 */
void apparentCoord(       SkyPoint   *p,
                    const JulianDate  jd0,
                    const JulianDate  jdf );

/** Determine the effects of nutation for this SkyPoint.
 *
 *  @param p pointer to the SkyPoint in question
 *  @param num pointer to KSNumbers object containing current values of
 *         time-dependent variables.
 */
void nutate(       SkyPoint  *p,
             const KSNumbers *num );


/** Check if this sky point is close enough to the sun for gravitational 
 *  lensing to be significant.
 *
 *  @param p the point in question
 *  @param sun the sun.
 *  @return true if it is close enough
 */
bool checkBendLight( const SkyPoint *p,
                     const KSSun    *sun );

/** Correct for the effect of "bending" of light around the sun for
 *  positions near the sun.
 *
 *  General Relativity tells us that a photon with an impact
 *  parameter b is deflected through an angle 1.75" (Rs / b) where
 *  Rs is the solar radius.
 *
 *  @param p the point in question
 *  @param sun the sun
 */
void bendlight(       SkyPoint *p,
                const KSSun    *sun );


} // NS OldPointfunctions
} // NS KSEngine

#endif //KSENGINE_OLDPOINTFUNCTIONS_H
