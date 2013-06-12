/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Henry de Valence <hdevalence@hdevalence.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "oldvlsr.h"

#include "skypoint.h"
#include "ksnumbers.h"

namespace KSEngine {
namespace OldVLSR {

double vRSun(const SkyPoint &s, const JulianDate jd)
{
    double ca, sa, cd, sd, vsun;
    double cosRA, sinRA, cosDec, sinDec;

    /* Sun apex (where the sun goes) coordinates */

    dms asun(270.9592); // Right ascention: 18h 3m 50.2s [J2000]
    dms dsun(30.00467); // Declination: 30^o 0' 16.8'' [J2000]
    vsun=20.;           // [km/s]

    asun.SinCos(sa,ca);
    dsun.SinCos(sd,cd);

    /* We need an auxiliary SkyPoint since we need the
    * source referred to the J2000 equinox and we do not want to ovewrite
    * the current values
    */

    SkyPoint aux;
    aux.set(s.ra0(), s.dec0());

    aux.precessFromAnyEpoch(jd, EpochJ2000);

    aux.ra().SinCos( sinRA, cosRA );
    aux.dec().SinCos( sinDec, cosDec );

    /* Computation is done performing the scalar product of a unitary vector
    in the direction of the source with the vector velocity of Sun, both being in the
    LSR reference system:
    Vlsr    = Vhel + Vsun.u_radial =>
    Vlsr    = Vhel + vsun(cos D cos A,cos D sen A,sen D).(cos d cos a,cos d sen a,sen d)
    Vhel    = Vlsr - Vsun.u_radial
    */

    return vsun *(cd*cosDec*(cosRA*ca + sa*sinRA) + sd*sinDec);
}

double vHeliocentric(const SkyPoint &s, const double vlsr, const JulianDate jd)
{
    return vlsr - vRSun(s,jd);
}

double vHelioToVlsr(const SkyPoint &s, const double vhelio, const JulianDate jd)
{
    return vhelio + vRSun(s,jd);
}

double vREarth(const SkyPoint &s, const JulianDate jd)
{
    double sinRA, sinDec, cosRA, cosDec;

    /* u_radial = unitary vector in the direction of the source
        Vlsr    = Vhel + Vsun.u_radial
            = Vgeo + VEarth.u_radial + Vsun.u_radial  =>

        Vgeo    = (Vlsr -Vsun.u_radial) - VEarth.u_radial
            =  Vhel - VEarth.u_radial
            =  Vhel - (vx, vy, vz).(cos d cos a,cos d sen a,sen d)
    */

    /* We need an auxiliary SkyPoint since we need the
    * source referred to the J2000 equinox and we do not want to ovewrite
    * the current values
    */

    SkyPoint aux(s.ra0(), s.dec0());

    aux.precessFromAnyEpoch(jd, EpochJ2000);

    aux.ra().SinCos( sinRA, cosRA );
    aux.dec().SinCos( sinDec, cosDec );

    /* vEarth is referred to the J2000 equinox, hence we need that
    the source coordinates are also in the same reference system.
    */

    KSNumbers num(jd);
    return num.vEarth(0) * cosDec * cosRA +
           num.vEarth(1) * cosDec * sinRA +
           num.vEarth(2) * sinDec;
}

double vGeocentric(const SkyPoint &s, const double vhelio, const JulianDate jd)
{
    return vhelio - vREarth(s,jd);
}

double vGeoToVHelio(const SkyPoint &s, const double vgeo, const JulianDate jd)
{
    return vgeo + vREarth(s,jd);
}

double vRSite(const SkyPoint &s, const double vsite[3])
{
    double sinRA, sinDec, cosRA, cosDec;

    s.ra().SinCos( sinRA, cosRA );
    s.dec().SinCos( sinDec, cosDec );

    return vsite[0]*cosDec*cosRA + vsite[1]*cosDec*sinRA + vsite[2]*sinDec;
}

double vTopoToVGeo(const SkyPoint &s, const double vtopo, const double vsite[3])
{
    return vtopo + vRSite(s,vsite);
}

double vTopocentric(const SkyPoint &s, const double vgeo, const double vsite[3])
{
    return vgeo - vRSite(s,vsite);
}

} //NS OldVLSR
} //NS KSEngine