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

#include "engine/oldpointfunctions.h"

namespace KSEngine {
namespace OldPointFunctions {

void updateCoords(SkyPoint* p, const KSNumbers* num, const bool forceRecompute)
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
        nutate(p,num);
        if( lens )
            p->bendlight();
        p->aberrate(num);
        p->lastPrecessJD = num->getJD();
    }
}

void apparentCoord(SkyPoint* p, const JulianDate jd0, const JulianDate jdf)
{
    p->precessFromAnyEpoch(jd0,jdf);
    KSNumbers num(jdf);
    nutate(p,&num);
    if( Options::useRelativistic() && p->checkBendLight() )
        p->bendlight();
    p->aberrate( &num );
}

void nutate(SkyPoint* p, const KSNumbers* num)
{
    double cosRA, sinRA, cosDec, sinDec, tanDec;
    double cosOb, sinOb;

    dms RA  = p->ra();
    dms Dec = p->dec();

    RA.SinCos( sinRA, cosRA );
    Dec.SinCos( sinDec, cosDec );

    num->obliquity()->SinCos( sinOb, cosOb );

    //Step 2: Nutation
    if ( fabs( Dec.Degrees() ) < 80.0 ) { //approximate method
        tanDec = sinDec/cosDec;

        double dRA  = num->dEcLong()*( cosOb + sinOb*sinRA*tanDec ) - num->dObliq()*cosRA*tanDec;
        double dDec = num->dEcLong()*( sinOb*cosRA ) + num->dObliq()*sinRA;

        RA.setD( RA.Degrees() + dRA );
        Dec.setD( Dec.Degrees() + dDec );

        p->setRA(RA);
        p->setDec(Dec);
    } else { //exact method
        dms EcLong, EcLat;
        p->findEcliptic( num->obliquity(), EcLong, EcLat );

        //Add dEcLong to the Ecliptic Longitude
        dms newLong( EcLong.Degrees() + num->dEcLong() );
        p->setFromEcliptic( num->obliquity(), newLong, EcLat );
    }
}

} // NS OldPointfunctions
} // NS KSEngine