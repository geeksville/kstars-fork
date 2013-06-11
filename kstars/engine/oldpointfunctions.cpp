/***************************************************************************
  engine/oldpointfunctions.cpp - deprecated functions for manipulating points.
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

#include "oldpointfunctions.h"


#include "Options.h"
#include "skypoint.h"
#include "ksnumbers.h"
#include "dms.h"

namespace KSEngine {
namespace OldPointFunctions {

void updateCoords(SkyPoint* p, KSNumbers* num, bool forceRecompute)
{
    //Correct the catalog coordinates for the time-dependent effects
    //of precession, nutation and aberration
    bool recompute, lens;

    // NOTE: The same short-circuiting checks are also implemented in
    // StarObject::JITUpdate(), even before calling
    // updateCoords(). While this is code-duplication, these bits of
    // code need to be really optimized, at least for stars. For
    // optimization purposes, the code is left duplicated in two
    // places. Please be wary of changing one without changing the
    // other.

    Q_ASSERT( std::isfinite( p->lastPrecessJD ) );

    if( Options::useRelativistic() && p->checkBendLight() ) {
        recompute = true;
        lens = true;
    }
    else {
        recompute = ( Options::alwaysRecomputeCoordinates() ||
                      ( p->lastPrecessJD - num->getJD() ) >=  0.0005 ||
                      ( p->lastPrecessJD - num->getJD() ) <= -0.0005 || forceRecompute ); // 0.0005 solar days is less than a minute
        lens = false;
    }
    if( recompute ) {
        p->precess(num);
        p->nutate(num);
        if( lens )
            p->bendlight();
        p->aberrate(num);
        p->lastPrecessJD = num->getJD();
    }
}

void apparentCoord(SkyPoint* p, JulianDate jd0, JulianDate jdf)
{
    p->precessFromAnyEpoch(jd0,jdf);
    KSNumbers num(jdf);
    p->nutate( &num );
    if( Options::useRelativistic() && p->checkBendLight() )
        p->bendlight();
    p->aberrate( &num );
}

} // NS OldPointfunctions
} // NS KSEngine