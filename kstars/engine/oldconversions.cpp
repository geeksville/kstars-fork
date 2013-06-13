/***************************************************************************
  engine/oldconversions.cpp - deprecated functions for converting coordinates.
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

#include "oldconversions.h"

#include <kdebug.h>

#include "dms.h"
#include "skypoint.h"

namespace KSEngine {
namespace OldConversions {

void EquatorialToHorizontal(SkyPoint *p, const dms *LST, const dms *lat)
{
    //Uncomment for spherical trig version
    double AltRad, AzRad;
    double sindec, cosdec, sinlat, coslat, sinHA, cosHA;
    double sinAlt, cosAlt;

    dms HourAngle = (*LST) - p->ra();

    lat->SinCos( sinlat, coslat );
    p->dec().SinCos( sindec, cosdec );
    HourAngle.SinCos( sinHA, cosHA );

    sinAlt = sindec*sinlat + cosdec*coslat*cosHA;
    AltRad = asin( sinAlt );
    cosAlt = cos( AltRad );

    double arg = ( sindec - sinlat*sinAlt )/( coslat*cosAlt );
    if ( arg <= -1.0 ) AzRad = dms::PI;
    else if ( arg >= 1.0 ) AzRad = 0.0;
    else AzRad = acos( arg );

    if ( sinHA > 0.0 ) AzRad = 2.0*dms::PI - AzRad; // resolve acos() ambiguity

    dms tmp;
    tmp.setRadians( AltRad );
    p->setAlt( tmp );
    tmp.setRadians( AzRad );
    p->setAz( tmp );

    // //Uncomment for XYZ version
    //      double xr, yr, zr, xr1, zr1, sa, ca;
    //  //Z-axis rotation by -LST
    //  dms a = dms( -1.0*LST->Degrees() );
    //  a.SinCos( sa, ca );
    //  xr1 = m_X*ca + m_Y*sa;
    //  yr  = -1.0*m_X*sa + m_Y*ca;
    //  zr1 = m_Z;
    //
    //  //Y-axis rotation by lat - 90.
    //  a = dms( lat->Degrees() - 90.0 );
    //  a.SinCos( sa, ca );
    //  xr = xr1*ca - zr1*sa;
    //  zr = xr1*sa + zr1*ca;
    //
    //  //FIXME: eventually, we will work with XYZ directly
    //  Alt.setRadians( asin( zr ) );
    //  Az.setRadians( atan2( yr, xr ) );
}

void HorizontalToEquatorial(SkyPoint *p, const dms *LST, const dms *lat)
{
    double HARad, DecRad;
    double sinlat, coslat, sinAlt, cosAlt, sinAz, cosAz;
    double sinDec, cosDec;

    lat->SinCos( sinlat, coslat );
    p->alt().SinCos( sinAlt, cosAlt );
    p->az().SinCos( sinAz,  cosAz );

    sinDec = sinAlt*sinlat + cosAlt*coslat*cosAz;
    DecRad = asin( sinDec );
    cosDec = cos( DecRad );
    dms Dec = p->dec();
    Dec.setRadians( DecRad );
    p->setDec(Dec);

    double x = ( sinAlt - sinlat*sinDec )/( coslat*cosDec );

    //Under certain circumstances, x can be very slightly less than -1.0000, or slightly
    //greater than 1.0000, leading to a crash on acos(x).  However, the value isn't
    //*really* out of range; it's a kind of roundoff error.
    if ( x < -1.0 && x > -1.000001 ) HARad = dms::PI;
    else if ( x > 1.0 && x < 1.000001 ) HARad = 0.0;
    else if ( x < -1.0 ) {
        kWarning() << "Coordinate out of range." << endl;
        HARad = dms::PI;
    } else if ( x > 1.0 ) {
        kWarning() << "Coordinate out of range." << endl;
        HARad = 0.0;
    } else HARad = acos( x );

    if ( sinAz > 0.0 ) HARad = 2.0*dms::PI - HARad; // resolve acos() ambiguity

    dms RA = p->ra();
    RA.setRadians( LST->radians() - HARad );
    RA.setD( RA.reduce().Degrees() );  // 0 <= RA < 24
    p->setRA( RA );
}

void findEcliptic(const SkyPoint *p, const dms *Obliquity, dms &EcLong, dms &EcLat)
{
    double sinRA, cosRA, sinOb, cosOb, sinDec, cosDec, tanDec;
    p->ra().SinCos( sinRA, cosRA );
    p->dec().SinCos( sinDec, cosDec );
    Obliquity->SinCos( sinOb, cosOb );

    tanDec = sinDec/cosDec;                    // FIXME: -jbb div by zero?
    double y = sinRA*cosOb + tanDec*sinOb;
    double ELongRad = atan2( y, cosRA );
    EcLong.setRadians( ELongRad );
    EcLong.reduce();
    EcLat.setRadians( asin( sinDec*cosOb - cosDec*sinOb*sinRA ) );
}

void setFromEcliptic(SkyPoint *p, const dms *Obliquity, const dms &EcLong, const dms &EcLat)
{
    double sinLong, cosLong, sinLat, cosLat, sinObliq, cosObliq;
    EcLong.SinCos( sinLong, cosLong );
    EcLat.SinCos( sinLat, cosLat );
    Obliquity->SinCos( sinObliq, cosObliq );

    double sinDec = sinLat*cosObliq + cosLat*sinObliq*sinLong;

    double y = sinLong*cosObliq - (sinLat/cosLat)*sinObliq;
    double RARad =  atan2( y, cosLong );

    dms RA = p->ra();
    dms Dec = p->dec();

    RA.setRadians( RARad );
    RA.reduce();
    Dec.setRadians( asin(sinDec) );

    p->setRA(RA);
    p->setDec(Dec);
}

void Equatorial1950ToGalactic(const SkyPoint *p, dms &galLong, dms &galLat)
{
    dms RA = p->ra();
    dms Dec = p->dec();

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

void GalacticToEquatorial1950(SkyPoint *p, const dms *galLong, const dms *galLat)
{
    double a = 123.0;
    double sinb, cosb, singLat, cosgLat, tangLat, singLong_a, cosgLong_a;

    dms c(12.25);
    dms b(27.4);
    tangLat = tan( galLat->radians() );
    galLat->SinCos(singLat,cosgLat);

    dms( galLong->Degrees() - a ).SinCos(singLong_a,cosgLong_a);
    b.SinCos(sinb,cosb);

    dms RA = p->ra();
    dms Dec = p->dec();

    RA.setRadians(c.radians() + atan2(singLong_a,cosgLong_a*sinb-tangLat*cosb) );
    RA = RA.reduce();
    Dec.setRadians( asin(singLat*sinb+cosgLat*cosb*cosgLong_a) );

    p->setRA(RA);
    p->setDec(Dec);
}



}
}
