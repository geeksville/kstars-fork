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
    Vector3f v(1,0,0);
    dms lat, lon;
    ConvertCoord::vectorToSpherical(v,&lat,&lon);
    // Need to cast to float for precision issues
    QCOMPARE( (float)lat.Degrees(), 0.0f );
    QCOMPARE( (float)lon.Degrees(), 90.0f );
}

void TestConvertCoord::testSphericalToVector()
{
    dms lat(0); dms lon(-45);
    Vector3f v = ConvertCoord::sphericalToVector(lat, lon);
    ConvertCoord::vectorToSpherical(v,&lat,&lon);
    QCOMPARE( (float)lat.Degrees(), 0.f  );
    QCOMPARE( (float)lon.Degrees(), -45.f );

    lat = dms(45); lon = dms(0);
    v = ConvertCoord::sphericalToVector(lat, lon);
    ConvertCoord::vectorToSpherical(v,&lat,&lon);
    QCOMPARE( (float)lat.Degrees(), 45.f );
    QCOMPARE( (float)lon.Degrees(), 0.f  );

    lat = dms(0); lon = dms(0);
    v = ConvertCoord::sphericalToVector(lat, lon);
    ConvertCoord::vectorToSpherical(v,&lat,&lon);
    QCOMPARE( (float)lat.Degrees(), 0.f );
    QCOMPARE( (float)lon.Degrees(), 0.f );

    lat = dms(45); lon = dms(45);
    v = ConvertCoord::sphericalToVector(lat, lon);
    ConvertCoord::vectorToSpherical(v,&lat,&lon);
    QCOMPARE( (float)lat.Degrees(), 45.f );
    QCOMPARE( (float)lon.Degrees(), 45.f );
}

void TestConvertCoord::testRotB1950ToGal()
{
    CoordConversion c = ConvertCoord::rotB1950ToGal();
    dms lat,lon;
    GalacticCoord g;

    g = c*ConvertCoord::sphericalToVector(27.4*DEG2RAD, 192.25*DEG2RAD);
    ConvertCoord::vectorToSpherical(g,&lat, &lon);
    QCOMPARE((float)lat.Degrees(), 90.f);

    g = c*ConvertCoord::sphericalToVector(-28.916790*DEG2RAD, 265.610844f*DEG2RAD);
    ConvertCoord::vectorToSpherical(g,&lat, &lon);
    QCOMPARE((float)lat.Degrees() + 1.f, 1.f); // discard 1 digit precision
    QCOMPARE((float)lon.Degrees() + 1.f, 1.f);

    g = c*ConvertCoord::sphericalToVector(-27.4*DEG2RAD, 12.25*DEG2RAD);
    ConvertCoord::vectorToSpherical(g,&lat, &lon);
    QCOMPARE((float)lat.Degrees(), -90.f);
}

QTEST_MAIN(TestConvertCoord)

#include "testconvertcoord.moc"

