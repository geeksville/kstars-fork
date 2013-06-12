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

} // NS OldPrecession
} // NS KSEngine

#endif //KSENGINE_OLDPRECESSION_H