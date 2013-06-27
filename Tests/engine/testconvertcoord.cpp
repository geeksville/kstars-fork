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

QTEST_MAIN(TestConvertCoord)

#include "testconvertcoord.moc"

