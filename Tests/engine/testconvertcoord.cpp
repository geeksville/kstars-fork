/***************************************************************************
              testconvertcoord.cpp - Tests coordinate conversions
                             -------------------
    begin                : 2013-06-20
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

#include "testconvertcoord.h"

#include "kstars/engine/convertcoord.h"
#include "kstars/dms.h"

using namespace KSEngine;

void TestConvertCoord::testVectorToSpherical()
{
    Vector3d v(1,0,0);
    dms lat, lon;
    Convert::vectToSph(v,&lat,&lon);
    // Need to cast to float for precision issues
    QCOMPARE( lat.Degrees(), 0. );
    QCOMPARE( lon.Degrees(), 90. );
}

void TestConvertCoord::testSphericalToVector()
{
    dms lat(0); dms lon(-45);
    Vector3d v = Convert::sphToVect(lat, lon);
    Convert::vectToSph(v,&lat,&lon);
    QCOMPARE( lat.Degrees(), 0.  );
    QCOMPARE( lon.Degrees(), -45. );

    lat = dms(45); lon = dms(0);
    v = Convert::sphToVect(lat, lon);
    Convert::vectToSph(v,&lat,&lon);
    QCOMPARE( lat.Degrees(), 45. );
    QCOMPARE( lon.Degrees(), 0.  );

    lat = dms(0); lon = dms(0);
    v = Convert::sphToVect(lat, lon);
    Convert::vectToSph(v,&lat,&lon);
    QCOMPARE( lat.Degrees(), 0. );
    QCOMPARE( lon.Degrees(), 0. );

    lat = dms(45); lon = dms(45);
    v = Convert::sphToVect(lat, lon);
    Convert::vectToSph(v,&lat,&lon);
    QCOMPARE( lat.Degrees(), 45. );
    QCOMPARE( lon.Degrees(), 45. );
}

void TestConvertCoord::testRotB1950ToGal()
{
    CoordConversion c = Convert::B1950ToGal();
    dms lat,lon;
    GalacticCoord g;

    g = c*Convert::sphToVect(27.4*DEG2RAD, 192.25*DEG2RAD);
    Convert::vectToSph(g,&lat, &lon);
    QCOMPARE(lat.Degrees(), 90.);

    g = c*Convert::sphToVect(-28.91679035*DEG2RAD, 265.61084403*DEG2RAD);
    Convert::vectToSph(g,&lat, &lon);
    QVERIFY( (lat.Degrees() - 0.) < MILLIARCSEC_DEGREES );
    QVERIFY( (lon.Degrees() - 0.) < MILLIARCSEC_DEGREES );

    g = c*Convert::sphToVect(-27.4*DEG2RAD, 12.25*DEG2RAD);
    Convert::vectToSph(g,&lat, &lon);
    QCOMPARE(lat.Degrees(), -90.);
}

void TestConvertCoord::testConvertEqToEcl()
{
    dms ra(20), dec(30);
    JulianDate jd = EpochJ2000 + 36525.;
    Vector3d result(0.470567674777011279996230541656,
                    0.341026974424810169761457245841,
                    0.813797681349373691617188342207);
    Vector3d v = Convert::EqToEcl(jd) * Convert::sphToVect(dec, ra);
    QVERIFY(result.isApprox(v));
}

void TestConvertCoord::testPrecession()
{
    //Precess 1 century into the future
    JulianDate jd = EpochJ2000 + 36525.;
    dms ra(20), dec(30);
    Vector3d result(0.314271323430725058045709374710,
                    0.507849575050525858799232992169,
                    0.802073777398376819292025174946);

    Vector3d v = Convert::PrecessTo(jd) * Convert::sphToVect(dec, ra);
    QVERIFY(result.isApprox(v));
}

void TestConvertCoord::testNutation()
{
    //Nutate 1 century into the future
    JulianDate jd = EpochJ2000 + 36525.;
    dms ra(20), dec(81);
    EquatorialCoord result(0.053505875240156049177375763293,
                           0.987689266559254708255366494996,
                           0.146993312903773154509678988688);
    EquatorialCoord v = Convert::Nutate(jd) * Convert::sphToVect(dec, ra);
    QVERIFY(result.isApprox(v));
}

void TestConvertCoord::testAberration()
{
    JulianDate jd = EpochJ2000 + 36525.;
    dms ra(20), dec(30);
    EquatorialCoord result(0.296206820559580585872083702270,
                           0.500037158377960588850896783697,
                           0.813771687695804990525516586786);
    EclipticCoord v = Convert::Aberrate( Convert::EqToEcl(jd)*Convert::sphToVect(dec,ra), jd );
    QVERIFY(result.isApprox(Convert::EclToEq(jd)*v, 1e-10));
}

void TestConvertCoord::testConvertEqToHor()
{
    dms lat(43.7), LST(50.0), ra(20), dec(30);
    HorizontalCoord result(-0.433012701892219353805302262117,
                            0.887666564970605254103475090233,
                           -0.156678235352859906992506466850);
    HorizontalCoord v = Convert::EqToHor(LST, lat) * Convert::sphToVect(dec, ra);
    QVERIFY(result.isApprox(v));
}

void TestConvertCoord::testFindPosition()
{
    // Pick a point with dec > 80 so that the old code uses the precise
    // method for nutation, which the new code uses always. This way
    // we don't pick up error from that approximation.
    dms ra(30), dec(80.1);
    dms lat(43.7), LST(50.0);
    JulianDate jd = EpochJ2000 + 36525.;
    HorizontalCoord result(-0.047837445506930538485779180746,
                            0.794725389083798638978350936668,
                            0.605081097666235523391264905513);

    J2000Coord v0 = Convert::sphToVect(dec,ra);
    EquatorialCoord pn = Convert::Nutate(jd) * Convert::PrecessTo(jd) * v0;
    EclipticCoord ab = Convert::Aberrate( Convert::EqToEcl(jd) * pn, jd );
    HorizontalCoord h = Convert::EqToHor( LST, lat ) * Convert::EclToEq(jd) * ab;

    //TODO: is this good enough? Should we try to improve the precision?
    //As far as I can see, most of this error comes from the aberration calculation.
    QVERIFY(result.isApprox(h,1e-7));
}

QTEST_MAIN(TestConvertCoord)

#include "testconvertcoord.moc"

