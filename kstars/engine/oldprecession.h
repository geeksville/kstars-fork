/***************************************************************************
  engine/oldprecession.h - deprecated functions for calculating precession.
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


#ifndef KSENGINE_OLDPRECESSION_H
#define KSENGINE_OLDPRECESSION_H

#include "types.h"

class SkyPoint;
class KSNumbers;

namespace KSEngine {
/**
 * This namespace contains unported versions of old precession code
 * that used to be in the Skypoint class.
 * Using these is very inefficient, so existing code should, over time,
 * be ported to use new APIs.
 */
namespace OldPrecession {

    /** Precess this SkyPoint's catalog coordinates to the epoch described 
     *  by the given KSNumbers object.
     *
     *  @param p the point in question
     *  @param num pointer to a KSNumbers object describing the target epoch.
     */
    void precess(       SkyPoint  *p,
                  const KSNumbers *num);

    /** General case of precession. It precess from an original epoch to a
     *  final epoch. In this case RA0, and Dec0 from SkyPoint object represent
     *  the coordinates for the original epoch and not for J2000, as usual.
     *
     *  FIXME: This implementation should be rewritten cleanly.
     *
     *  @param p the point in question
     *  @param jd0 Julian Day which identifies the original epoch
     *  @param jdf Julian Day which identifies the final epoch
     */
    void precessFromAnyEpoch(       SkyPoint   *p,
                                    JulianDate  jd0,
                                    JulianDate  jdf );

    /** Obtain a Skypoint with RA0 and Dec0 set from the RA, Dec
    *  of this skypoint. Also set the RA0, Dec0 of this SkyPoint if not
    *  set already.
    * 
    *  @param p the point in question
    *  @param num a whole bunch of extra data //FIXME clean this class out
    *  @param epoch the epoch to deprecess to
    */
    SkyPoint deprecess(       SkyPoint   *p,
                        const KSNumbers  *num,
                        const JulianDate  epoch=EpochJ2000 );

    /** Exact precession from Besselian epoch 1950 to epoch J2000. The
     *  coordinates referred to the first epoch are in the 
     *  FK4 catalog, while the latter are in the Fk5 one.
     *  Reference: Smith, C. A.; Kaplan, G. H.; Hughes, J. A.; Seidelmann,
     *  P. K.; Yallop, B. D.; Hohenkerk, C. Y.
     *  Astronomical Journal, vol. 97, Jan. 1989, p. 265-279
     *  This transformation requires 4 steps:
     *  - Correct E-terms
     *  - Precess from B1950 to 1984, January 1st, 0h, using Newcomb expressions
     *  - Add zero point correction in right ascension for 1984
     *  - Precess from 1984, January 1st, 0h to J2000
     *
     *  @param p the point to precess
     */
    void B1950ToJ2000( SkyPoint *p );

    /** Exact precession from epoch J2000 Besselian epoch 1950. The coordinates
     * referred to the first epoch are in the FK4 catalog, while the 
     * latter are in the Fk5 one.
     * Reference: Smith, C. A.; Kaplan, G. H.; Hughes, J. A.; Seidelmann,
     * P. K.; Yallop, B. D.; Hohenkerk, C. Y.
     * Astronomical Journal, vol. 97, Jan. 1989, p. 265-279
     * This transformation requires 4 steps:
     *  - Precess from J2000 to 1984, January 1st, 0h
     *  - Add zero point correction in right ascension for 1984
     *  - Precess from 1984, January 1st, 0h, to B1950 using Newcomb expressions
     *  - Correct E-terms
     *
     *  @param p the point to precess
     */
    void J2000ToB1950( SkyPoint *p );

} // NS OldPrecession
} // NS KSEngine

#endif //KSENGINE_OLDPRECESSION_H