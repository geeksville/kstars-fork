/***************************************************************************
  engine/oldprecession.cpp - deprecated functions for calculating precession.
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

#include "oldprecession.h"

#include <kdebug.h>

#include "skypoint.h"
#include "ksnumbers.h"

namespace {
    /** Determine the E-terms of aberration
     *  In the past, the mean places of stars published in catalogs included
     *  the contribution to the aberration due to the ellipticity of the orbit
     *  of the Earth. These terms, known as E-terms were almost constant, and
     *  in the newer catalogs (FK5) are not included. Therefore to convert from
     *  FK4 to FK5 one has to compute these E-terms.
     */
    SkyPoint Eterms(const SkyPoint *p) {
        double sd, cd, sinEterm, cosEterm;
        dms raTemp, raDelta, decDelta;

        p->dec().SinCos(sd,cd);
        raTemp.setH( p->ra().Hours() + 11.25);
        raTemp.SinCos(sinEterm,cosEterm);

        raDelta.setH( 0.0227*sinEterm/(3600.*cd) );
        decDelta.setD( 0.341*cosEterm*sd/3600. + 0.029*cd/3600. );

        return SkyPoint(raDelta, decDelta);
    };

    /** Coordinates in the FK4 catalog include the effect of aberration due
     *  to the ellipticity of the orbit of the Earth. Coordinates in the FK5
     *  catalog do not include these terms. In order to convert from B1950 (FK4)
     *  to actual mean places one has to use this function.
     */
    void addEterms(SkyPoint *p) {
        SkyPoint spd = Eterms(p);
        p->setRA( p->ra() + spd.ra() );
        p->setDec( p->dec() + spd.dec() );
    };

    /** Coordinates in the FK4 catalog include the effect of aberration due
     *  to the ellipticity of the orbit of the Earth. Coordinates in the FK5 
     *  catalog do not include these terms. In order to convert from 
     *  FK5 coordinates to B1950 (FK4) one has to use this function. 
     */
    void subtractEterms(SkyPoint *p) {
        SkyPoint spd = Eterms(p);
        p->setRA( p->ra() - spd.ra() );
        p->setDec( p->dec() - spd.dec() );
    };
}

namespace KSEngine {
namespace OldPrecession {

void precess(SkyPoint *p, const KSNumbers *num)
{
    double cosRA0, sinRA0, cosDec0, sinDec0;
    double v[3], s[3];
    dms RA = p->ra0();
    dms Dec = p->dec0();

    RA.SinCos( sinRA0, cosRA0 );
    Dec.SinCos( sinDec0, cosDec0 );

    s[0] = cosRA0*cosDec0;
    s[1] = sinRA0*cosDec0;
    s[2] = sinDec0;
    //Multiply P2 and s to get v, the vector representing the new coords.
    for ( unsigned int i=0; i<3; ++i ) {
        v[i] = 0.0;
        for (uint j=0; j< 3; ++j) {
            v[i] += num->p2( j, i )*s[j];
        }
    }

    //Extract RA, Dec from the vector:
    RA.setRadians( atan2( v[1], v[0] ) );
    RA.reduce();
    Dec.setRadians( asin( v[2] ) );
    //Update the point
    p->setRA( RA );
    p->setDec( Dec );
}

void precessFromAnyEpoch(SkyPoint *p, JulianDate jd0, JulianDate jdf)
{
    double cosRA, sinRA, cosDec, sinDec;
    double v[3], s[3];
    //We use the catalog coordinates, since we want
    //to set the current coords to the catalog coords.
    dms RA = p->ra0();
    dms Dec = p->dec0();

    p->setRA( RA );
    p->setDec( Dec );

    RA.SinCos( sinRA, cosRA );
    Dec.SinCos( sinDec, cosDec );

    if (jd0 == jdf)
        return;

    if (jd0 == EpochB1950) {
        B1950ToJ2000(p);
        jd0 = EpochJ2000;
        RA.SinCos( sinRA, cosRA );
        Dec.SinCos( sinDec, cosDec );
    }

    // The original coordinate is referred to the FK5 system and
    // is NOT J2000.
    if ( jd0 != EpochJ2000 ) {
        //v is a column vector representing input coordinates.
        v[0] = cosRA*cosDec;
        v[1] = sinRA*cosDec;
        v[2] = sinDec;

        //Need to first precess to J2000.0 coords
        //s is the product of P1 and v; s represents the
        //coordinates precessed to J2000
        KSNumbers num(jd0);
        for ( unsigned int i=0; i<3; ++i ) {
            s[i] = num.p1( 0, i )*v[0] +
                    num.p1( 1, i )*v[1] +
                    num.p1( 2, i )*v[2];
        }
        //Input coords already in J2000, set s accordingly.
    } else {
        s[0] = cosRA*cosDec;
        s[1] = sinRA*cosDec;
        s[2] = sinDec;
    }

    if ( jdf == EpochB1950) {
        RA.setRadians( atan2( s[1],s[0] ) );
        Dec.setRadians( asin( s[2] ) );
        p->setRA( RA );
        p->setDec( Dec );
        J2000ToB1950(p);
        return;
    }

    KSNumbers num(jdf);
    for ( unsigned int i=0; i<3; ++i ) {
        v[i] = num.p2( 0, i )*s[0] +
               num.p2( 1, i )*s[1] +
               num.p2( 2, i )*s[2];
    }

    RA.setRadians( atan2( v[1],v[0] ) );
    Dec.setRadians( asin( v[2] ) );

    if (RA.Degrees() < 0.0 )
        RA.setD( RA.Degrees() + 360.0 );

    p->setRA( RA );
    p->setDec( Dec );
}

SkyPoint deprecess(SkyPoint* p, const KSNumbers* num, const JulianDate epoch)
{
    SkyPoint p1( p->ra(), p->dec() );
    JulianDate now = num->julianDay();
    precessFromAnyEpoch( p, now, epoch );
    if( p->ra0().Degrees() < 0.0 || p->dec0().Degrees() > 90.0 ) 
    {
        kWarning() << "Invalid catalog coordinates in deprecess";
        // We have invalid RA0 and Dec0, so set them.
        p->setRA0(p1.ra());
        p->setDec0(p1.dec());
    }
    return p1;
}

void B1950ToJ2000(SkyPoint *p)
{
    double cosRA, sinRA, cosDec, sinDec;
    //  double cosRA0, sinRA0, cosDec0, sinDec0;
    double v[3], s[3];

    // 1984 January 1 0h
    // FIXME: this should probably be a named constant
    KSNumbers num(2445700.5);

    // Eterms due to aberration
    addEterms(p);
    dms RA = p->ra();
    dms Dec = p->dec();
    RA.SinCos( sinRA, cosRA );
    Dec.SinCos( sinDec, cosDec );

    // Precession from B1950 to J1984
    s[0] = cosRA*cosDec;
    s[1] = sinRA*cosDec;
    s[2] = sinDec;
    for ( unsigned int i=0; i<3; ++i ) {
        v[i] = num.p2b( 0, i )*s[0]
             + num.p2b( 1, i )*s[1]
             + num.p2b( 2, i )*s[2];
    }

    // RA zero-point correction at 1984 day 1, 0h.
    RA.setRadians( atan2( v[1],v[0] ) );
    Dec.setRadians( asin( v[2] ) );

    RA.setH( RA.Hours() + 0.06390/3600. );
    RA.SinCos( sinRA, cosRA );
    Dec.SinCos( sinDec, cosDec );

    s[0] = cosRA*cosDec;
    s[1] = sinRA*cosDec;
    s[2] = sinDec;

    // Precession from 1984 to J2000.

    for ( unsigned int i=0; i<3; ++i ) {
        v[i] = num.p1( 0, i )*s[0] +
               num.p1( 1, i )*s[1] +
               num.p1( 2, i )*s[2];
    }

    RA.setRadians( atan2( v[1],v[0] ) );
    Dec.setRadians( asin( v[2] ) );
    p->setRA(RA);
    p->setDec(Dec);
}

void J2000ToB1950(SkyPoint *p) {
    double cosRA, sinRA, cosDec, sinDec;
    //  double cosRA0, sinRA0, cosDec0, sinDec0;
    double v[3], s[3];

    // 1984 January 1 0h
    KSNumbers num(2445700.5);

    dms RA = p->ra();
    dms Dec = p->dec();
    RA.SinCos( sinRA, cosRA );
    Dec.SinCos( sinDec, cosDec );

    s[0] = cosRA*cosDec;
    s[1] = sinRA*cosDec;
    s[2] = sinDec;

    // Precession from J2000 to 1984 day, 0h.

    for ( unsigned int i=0; i<3; ++i ) {
        v[i] = num.p2( 0, i )*s[0] +
               num.p2( 1, i )*s[1] +
               num.p2( 2, i )*s[2];
    }

    RA.setRadians( atan2( v[1],v[0] ) );
    Dec.setRadians( asin( v[2] ) );

    // RA zero-point correction at 1984 day 1, 0h.

    RA.setH( RA.Hours() - 0.06390/3600. );
    RA.SinCos( sinRA, cosRA );
    Dec.SinCos( sinDec, cosDec );

    // Precession from B1950 to J1984

    s[0] = cosRA*cosDec;
    s[1] = sinRA*cosDec;
    s[2] = sinDec;
    for ( unsigned int i=0; i<3; ++i ) {
        v[i] = num.p1b( 0, i )*s[0]
             + num.p1b( 1, i )*s[1]
             + num.p1b( 2, i )*s[2];
    }

    RA.setRadians( atan2( v[1],v[0] ) );
    Dec.setRadians( asin( v[2] ) );

    //update point
    p->setRA( RA );
    p->setDec( Dec );
    // Eterms due to aberration
    subtractEterms(p);
}


} // NS OldPrecession
} // NS KSEngine