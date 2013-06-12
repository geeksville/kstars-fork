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
#include "kssun.h"
#include "kstarsdata.h"

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

    if( Options::useRelativistic() && checkBendLight(p,num->sun()) ) {
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
            bendlight(p,num->sun());
        p->aberrate(num);
        p->lastPrecessJD = num->getJD();
    }
}

void apparentCoord(SkyPoint* p, const JulianDate jd0, const JulianDate jdf)
{
    p->precessFromAnyEpoch(jd0,jdf);
    KSNumbers num(jdf);
    nutate(p,&num);
    if( Options::useRelativistic() && checkBendLight(p,num.sun()) )
        bendlight(p,num.sun());
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

bool checkBendLight(const SkyPoint* p, const KSSun* sun)
{
    // First see if we are close enough to the sun to bother about the
    // gravitational lensing effect. We correct for the effect at
    // least till b = 10 solar radii, where the effect is only about
    // 0.06".  Assuming min. sun-earth distance is 200 solar radii.
    static const dms maxAngle( 1.75 * ( 30.0 / 200.0) / dms::DegToRad );

    if (!sun) {
        return false;
    }

    // NOTE: dynamic_cast is slow and not important here.
    const SkyPoint *sunpt = static_cast<const SkyPoint *>(sun);

    // TODO: This can be optimized further. We only need a ballpark estimate of
    // the distance to the sun to start with.
    return ( fabs( p->angularDistanceTo( sunpt ).Degrees() ) <= maxAngle.Degrees() );
}

void bendlight(SkyPoint* p, const KSSun* sun)
{
    if (!sun) {
        return;
    }
    // NOTE: This should be applied before aberration
    // NOTE: One must call checkBendLight() before unnecessarily calling this.
    // We correct for GR effects
    const SkyPoint *sunpt = static_cast<const SkyPoint *>(sun);
    double corr_sec_top = 1.75 * sun->physicalSize();
    double corr_sec_bot = sun->rearth() * AU_KM * p->angularDistanceTo( sunpt ).sin();
    double corr_sec = corr_sec_top / corr_sec_bot;
    Q_ASSERT( corr_sec > 0 );

    SkyPoint sp = p->moveAway( *sun, corr_sec );
    p->setRA(  sp.ra() );
    p->setDec( sp.dec() );
}

} // NS OldPointfunctions
} // NS KSEngine