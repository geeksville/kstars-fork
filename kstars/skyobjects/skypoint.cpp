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
#include "dms.h"
#include "ksnumbers.h"
#include "kstarsdatetime.h" //for J2000 define
#include "kssun.h"
#include "kstarsdata.h"
#include "Options.h"
#include "skycomponents/skymapcomposite.h"

#include "engine/oldpointfunctions.h"

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

void SkyPoint::EquatorialToHorizontal( const dms *LST, const dms *lat ) {
    //Uncomment for spherical trig version
    double AltRad, AzRad;
    double sindec, cosdec, sinlat, coslat, sinHA, cosHA;
    double sinAlt, cosAlt;

    dms HourAngle = (*LST) - ra();

    lat->SinCos( sinlat, coslat );
    dec().SinCos( sindec, cosdec );
    HourAngle.SinCos( sinHA, cosHA );

    sinAlt = sindec*sinlat + cosdec*coslat*cosHA;
    AltRad = asin( sinAlt );
    cosAlt = cos( AltRad );

    double arg = ( sindec - sinlat*sinAlt )/( coslat*cosAlt );
    if ( arg <= -1.0 ) AzRad = dms::PI;
    else if ( arg >= 1.0 ) AzRad = 0.0;
    else AzRad = acos( arg );

    if ( sinHA > 0.0 ) AzRad = 2.0*dms::PI - AzRad; // resolve acos() ambiguity

    Alt.setRadians( AltRad );
    Az.setRadians( AzRad );

    // //Uncomment for XYZ version
    //  	double xr, yr, zr, xr1, zr1, sa, ca;
    // 	//Z-axis rotation by -LST
    // 	dms a = dms( -1.0*LST->Degrees() );
    // 	a.SinCos( sa, ca );
    // 	xr1 = m_X*ca + m_Y*sa;
    // 	yr  = -1.0*m_X*sa + m_Y*ca;
    // 	zr1 = m_Z;
    //
    // 	//Y-axis rotation by lat - 90.
    // 	a = dms( lat->Degrees() - 90.0 );
    // 	a.SinCos( sa, ca );
    // 	xr = xr1*ca - zr1*sa;
    // 	zr = xr1*sa + zr1*ca;
    //
    // 	//FIXME: eventually, we will work with XYZ directly
    // 	Alt.setRadians( asin( zr ) );
    // 	Az.setRadians( atan2( yr, xr ) );

}

void SkyPoint::HorizontalToEquatorial( const dms *LST, const dms *lat ) {
    double HARad, DecRad;
    double sinlat, coslat, sinAlt, cosAlt, sinAz, cosAz;
    double sinDec, cosDec;

    lat->SinCos( sinlat, coslat );
    alt().SinCos( sinAlt, cosAlt );
    Az.SinCos( sinAz,  cosAz );

    sinDec = sinAlt*sinlat + cosAlt*coslat*cosAz;
    DecRad = asin( sinDec );
    cosDec = cos( DecRad );
    Dec.setRadians( DecRad );

    double x = ( sinAlt - sinlat*sinDec )/( coslat*cosDec );

    //Under certain circumstances, x can be very slightly less than -1.0000, or slightly
    //greater than 1.0000, leading to a crash on acos(x).  However, the value isn't
    //*really* out of range; it's a kind of roundoff error.
    if ( x < -1.0 && x > -1.000001 ) HARad = dms::PI;
    else if ( x > 1.0 && x < 1.000001 ) HARad = 0.0;
    else if ( x < -1.0 ) {
        kWarning() << i18n( "Coordinate out of range." ) << endl;
        HARad = dms::PI;
    } else if ( x > 1.0 ) {
        kWarning() << i18n( "Coordinate out of range." ) << endl;
        HARad = 0.0;
    } else HARad = acos( x );

    if ( sinAz > 0.0 ) HARad = 2.0*dms::PI - HARad; // resolve acos() ambiguity

    RA.setRadians( LST->radians() - HARad );
    RA.setD( RA.reduce().Degrees() );  // 0 <= RA < 24
}

void SkyPoint::findEcliptic( const dms *Obliquity, dms &EcLong, dms &EcLat ) {
    double sinRA, cosRA, sinOb, cosOb, sinDec, cosDec, tanDec;
    ra().SinCos( sinRA, cosRA );
    dec().SinCos( sinDec, cosDec );
    Obliquity->SinCos( sinOb, cosOb );

    tanDec = sinDec/cosDec;                    // FIXME: -jbb div by zero?
    double y = sinRA*cosOb + tanDec*sinOb;
    double ELongRad = atan2( y, cosRA );
    EcLong.setRadians( ELongRad );
    EcLong.reduce();
    EcLat.setRadians( asin( sinDec*cosOb - cosDec*sinOb*sinRA ) );
}

void SkyPoint::setFromEcliptic( const dms *Obliquity, const dms& EcLong, const dms& EcLat ) {
    double sinLong, cosLong, sinLat, cosLat, sinObliq, cosObliq;
    EcLong.SinCos( sinLong, cosLong );
    EcLat.SinCos( sinLat, cosLat );
    Obliquity->SinCos( sinObliq, cosObliq );

    double sinDec = sinLat*cosObliq + cosLat*sinObliq*sinLong;

    double y = sinLong*cosObliq - (sinLat/cosLat)*sinObliq;
    double RARad =  atan2( y, cosLong );
    RA.setRadians( RARad );
    RA.reduce();
    Dec.setRadians( asin(sinDec) );
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

void SkyPoint::Equatorial1950ToGalactic(dms &galLong, dms &galLat) {

    double a = 192.25;
    double sinb, cosb, sina_RA, cosa_RA, sinDEC, cosDEC, tanDEC;

    dms c(303.0);
    dms b(27.4);
    tanDEC = tan( Dec.radians() );

    b.SinCos(sinb,cosb);
    dms( a - RA.Degrees() ).SinCos(sina_RA,cosa_RA);
    Dec.SinCos(sinDEC,cosDEC);

    galLong.setRadians( c.radians() - atan2( sina_RA, cosa_RA*sinb-tanDEC*cosb) );
    galLong = galLong.reduce();

    galLat.setRadians( asin(sinDEC*sinb+cosDEC*cosb*cosa_RA) );
}

void SkyPoint::GalacticToEquatorial1950(const dms* galLong, const dms* galLat) {

    double a = 123.0;
    double sinb, cosb, singLat, cosgLat, tangLat, singLong_a, cosgLong_a;

    dms c(12.25);
    dms b(27.4);
    tangLat = tan( galLat->radians() );
    galLat->SinCos(singLat,cosgLat);

    dms( galLong->Degrees() - a ).SinCos(singLong_a,cosgLong_a);
    b.SinCos(sinb,cosb);

    RA.setRadians(c.radians() + atan2(singLong_a,cosgLong_a*sinb-tangLat*cosb) );
    RA = RA.reduce();

    Dec.setRadians( asin(singLat*sinb+cosgLat*cosb*cosgLong_a) );
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
