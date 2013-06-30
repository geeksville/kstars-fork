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

#include "types.h"

#include <Eigen/Core>

namespace KSEngine {
namespace AstroVars {

    double centuriesSinceJ2000( const JulianDate jd );
    double sunMeanLongitude( const JulianDate jd );
    double meanElongationOfMoon( const JulianDate jd );
    double sunMeanAnomaly( const JulianDate jd );
    double moonMeanAnomaly( const JulianDate jd );
    double moonMeanLongitude( const JulianDate jd );
    double moonArgumentOfLatitude( const JulianDate jd );
    double lonMoonAscendingNode( const JulianDate jd );

    void nutationVars( const JulianDate  jd,
                             double     *deltaEcLong,
                             double     *deltaObliquity );

} // NS AstroVars
} // NS KSEngine

#endif //ASTROVARS_H

