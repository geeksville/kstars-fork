/***************************************************************************
     engine/astrovars.h - functions for use in various calculations
                             -------------------
    begin                : 2013-06-28
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

#ifndef ASTROVARS_H
#define ASTROVARS_H

#include "ksengine/kstypes.h"

namespace KSEngine {
namespace AstroVars {

    /**
     * Ecliptic longitude of earth's perhelion.
     * See commit 75515fa2b0a4cc4d8dc5e62261595350ac1f2311 for discussion.
     */
    static const Radian EarthPerhelionLongitude   = 102.94719*DEG2RAD;
    static const Radian EarthConstantOfAberration = 20.49552*ARCSEC2RAD;
    double centuriesSinceJ2000                  ( const JulianDate jd );
    double earthEccentricity                    ( const JulianDate jd );
    Radian meanElongationOfMoon                 ( const JulianDate jd );
    Radian sunMeanAnomaly                       ( const JulianDate jd );
    Radian sunMeanLongitude                     ( const JulianDate jd );
    Radian sunTrueLongitude                     ( const JulianDate jd );
    /** Meeus, Ch. 25. */
    Radian sunEquationOfCenter                  ( const JulianDate jd );
    Radian moonMeanAnomaly                      ( const JulianDate jd );
    Radian moonArgumentOfLatitude               ( const JulianDate jd );
    Radian lonMoonAscendingNode                 ( const JulianDate jd );

    /** @return the velocity vector of the earth at the given day. */
    Vector3d earthVelocity                      ( const JulianDate jd );

    void nutationVars( const JulianDate  jd,
                             double     *deltaEcLong,
                             double     *deltaObliquity );

} // NS AstroVars
} // NS KSEngine

#endif //ASTROVARS_H

