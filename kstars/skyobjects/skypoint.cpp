/***************************************************************************
                          skypoint.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001-2005 by Jason Harris
    email                : jharris@30doradus.org
    copyright            : (C) 2004-2005 by Pablo de Vicente
    email                : p.devicente@wanadoo.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "skypoint.h"

#include <kdebug.h>
#include <klocale.h>

#include "skyobject.h"
#include "ksengine/dms.h"
#include "ksnumbers.h"
#include "kstarsdatetime.h" //for J2000 define
#include "kssun.h"
#include "kstarsdata.h"
#include "Options.h"
#include "skycomponents/skymapcomposite.h"

#include "oldengine/oldpointfunctions.h"

SkyPoint::SkyPoint() {
    // Default constructor. Set nonsense values
    RA0.setD(-1); // RA >= 0 always :-)
    Dec0.setD(180); // Dec is between -90 and 90 Degrees :-)
    RA = RA0;
    Dec = Dec0;
    lastPrecessJD = J2000; // By convention, we use J2000 coordinates
}

void SkyPoint::set( const dms& r, const dms& d ) {
    RA0  = RA  = r;
    Dec0 = Dec = d;
    lastPrecessJD = J2000; // By convention, we use J2000 coordinates
}

SkyPoint::~SkyPoint(){
}

SkyPoint SkyPoint::moveAway( const SkyPoint &from, double dist ){
    dms lat1, dtheta;

    if( dist == 0.0 ) {
        kDebug() << "moveThrough called with zero distance!";
        return *this;
    }

    double dst = fabs( dist * dms::DegToRad / 3600.0 ); // In radian

    // Compute the bearing angle w.r.t. the RA axis ("latitude")
    dms dRA(  ra()  - from.ra()  );
    dms dDec( dec() - from.dec() );
    double bearing = atan2( dRA.sin() / dRA.cos(), dDec.sin() ); // Do not use dRA = PI / 2!!
    //double bearing = atan2( dDec.radians() , dRA.radians() );

    double dir0 = (dist >= 0 ) ? bearing : bearing + dms::PI; // in radian
    dist = fabs( dist ); // in radian


    lat1.setRadians( asin( dec().sin() * cos( dst ) +
                           dec().cos() * sin( dst ) * cos( dir0 ) ) );
    dtheta.setRadians( atan2( sin( dir0 ) * sin( dst ) * dec().cos(),
                              cos( dst ) - dec().sin() * lat1.sin() ) );

    return SkyPoint( ra() + dtheta, lat1 );
}

// Note: This method is one of the major rate determining factors in how fast the map pans / zooms in or out
void SkyPoint::updateCoords( KSNumbers *num, bool /*includePlanets*/, const dms *lat, const dms *LST, bool forceRecompute ) {
    //FIXME: we should rip out this whole method, but the fact that it's
    //virtual creates problems. It shouldn't be virtual anyways.
    // -- hdevalence, 2013-06-11
    KSEngine::OldPointFunctions::updateCoords(this, num, forceRecompute);

    if ( lat || LST )
        kWarning() << i18n( "lat and LST parameters should only be used in KSPlanetBase objects." ) ;
}

dms SkyPoint::angularDistanceTo(const SkyPoint *sp, double * const positionAngle) const {

    double dalpha = ra().radians() - sp->ra().radians() ;
    double ddelta = dec().radians() - sp->dec().radians() ;

    double sa = sin(dalpha/2.);
    double sd = sin(ddelta/2.);

    double hava = sa*sa;
    double havd = sd*sd;

    double aux = havd + cos (dec().radians()) * cos(sp->dec().radians()) // Haversine law
                 * hava;

    dms angDist;
    angDist.setRadians( 2 * fabs(asin( sqrt(aux) )) );

    if( positionAngle ) {
        // Also compute the position angle of the line from this SkyPoint to sp
        *positionAngle = acos( tan(-ddelta)/tan( angDist.radians() ) ); // FIXME: Might fail for large ddelta / zero angDist
        if( -dalpha < 0 )
            *positionAngle = 2*M_PI - *positionAngle;
    }
    return angDist;
}


bool SkyPoint::checkCircumpolar( const dms *gLat ) {
    return fabs(dec().Degrees())  >  (90 - fabs(gLat->Degrees()));
}
