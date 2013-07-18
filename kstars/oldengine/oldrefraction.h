/***************************************************************************
     engine/oldrefract.h - deprecated functions for atmo refraction.
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


#ifndef KSENGINE_OLDREFRACTION_H
#define KSENGINE_OLDREFRACTION_H

#include "ksengine/kstypes.h"
#include "ksengine/dms.h"

class SkyPoint;

namespace KSEngine {
/**
 * This namespace contains unported versions of old code for dealing with
 * refraction that used to be in the Skypoint class
 * Using these is very inefficient, so existing code should, over time,
 * be ported to use new APIs.
 */
namespace OldRefraction {

    /** @short Critical height for atmospheric refraction
     *  corrections. Below this, the height formula produces meaningles
     *  results and the correction value is just interpolated.
     */
    static const double altCrit = -1.0;

    /** Find the refracted altitude.
     *  This function uses Option::useRefraction to determine whether 
     *  refraction correction should be applied.
     *
     *  @param p the point to work with
     *  @return the refracted altitude.
     */
    dms altRefracted(const SkyPoint *p);

    /** Calculate refraction correction. 
     * 
     *  @param alt the altitude in degrees.
     *  @return the correction in degrees.
     */
    double refractionCorr(const double alt);

    /** Apply refraction correction to altitude.
     * 
     *  @param alt altitude to be corrected, in degrees
     *  @return altitude after refraction correction, in degrees
     */
    double refract(const double alt);

    /** @short Remove refraction correction.
     *  @param alt altitude from which refraction correction must be 
     *         removed, in degrees
     *  @return altitude without refraction correction, in degrees
     */
    double unrefract(const double alt);

    /**
     * @short Apply refraction correction to altitude. Overloaded method using
     * dms provided for convenience
     * @see SkyPoint::refract( const double alt )
     */
    inline dms refract(const dms alt) {
        return dms( refract( alt.Degrees() ) );
    }

    /**
     * @short Remove refraction correction. Overloaded method using
     * dms provided for convenience
     * @see SkyPoint::unrefract( const double alt )
     */
    inline dms unrefract(const dms alt) {
        return dms( unrefract( alt.Degrees() ) );
    }

} //NS OldRefraction
} //NS KSEngine

#endif //KSENGINE_OLDREFRACTION_H