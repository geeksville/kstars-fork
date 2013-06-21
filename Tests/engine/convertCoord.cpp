/***************************************************************************
                      convertCoord.cpp - KStars Tests
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

#include "convertCoord.h"

#include "kstars/engine/convertcoord.h"
#include "kstars/dms.h"

using namespace KSEngine;

/*
// cout gave a bunch of weird issues, and an > 1000 line compile error.
// Looked like something with Qt -- stdc++ interaction. We just want the values.
void printQ(Quaternionf q) {
    printf("q= %f %f %f %f\n\n\n", q.w(), q.x(), q.y(), q.z());
}
*/

void TestConvertCoord::testQuaternionToSpherical()
{
    //Manually construct a quaternion.
    Quaternionf q(AngleAxisf(0.5*M_PI, Vector3f::UnitZ()));
    //printQ(q);
    dms lat, lon;
    ConvertCoord::quaternionToSpherical(q,&lat,&lon);
    // Need to cast to float for precision issues
    QCOMPARE( (float)lat.Degrees(), 0.0f );
    QCOMPARE( (float)lon.Degrees(), 90.0f );
}

void TestConvertCoord::testSphericalToQuaternion()
{
    //see remarks above
    dms lat(0); dms lon(90);
    Quaternionf q = ConvertCoord::sphericalToQuaternion(lat, lon);
    Quaternionf qref(AngleAxisf(0.5*M_PI, Vector3f::UnitZ()));
    //printQ(q);
    //printQ(qref);
    QVERIFY(q.isApprox(qref));
}

QTEST_MAIN(TestConvertCoord)

#include "convertCoord.moc"
