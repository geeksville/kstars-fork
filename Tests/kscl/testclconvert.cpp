/***************************************************************************
 *         testclconvert.cpp - tests conversion using OpenCl
 *                             -------------------
 *    begin                : 2013-07-15
 *    copyright            : (C) 2013 by Henry de Valence
 *    email                : hdevalence@hdevalence.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "testclconvert.h"

// Eigen
#include <Eigen/Core>
using namespace Eigen;

// Qt
#include <QtCore/QVector>

// KDE
#include <KStandardDirs>
#include <KGlobal>

// Local
#include "kscl/ksclcontext.h"
#include "kscl/ksclbuffer.h"
#include "kstars/engine/oldconversions.h"
#include "kstars/engine/oldprecession.h"
#include "kstars/engine/oldpointfunctions.h"
#include "kstars/engine/convertcoord.h"
#include "kstars/ksnumbers.h"
#include "kstars/skyobjects/skypoint.h"
using namespace KSEngine;

void TestClConvert::initTestCase()
{
    // We need to modify the directories, since we aren't running
    // as KStars itself, so it won't search the right places.
    KGlobal::dirs()->addResourceType("appdata","data","kstars");
}

void TestClConvert::testApparentCoord()
{
    QTime t;
    
    // Parameters for the testing
    static const int NUM_TEST_POINTS = 1024*1024;
    dms ra(10), dec(81);
    dms lat(43.7), LST(50.0);
    JulianDate jd = EpochJ2000 + 36525.0;

    // Test with the old spherical trig method
    QVector<SkyPoint> skypoints(NUM_TEST_POINTS);
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        skypoints[i] = SkyPoint(ra,dec);
    }
    KSNumbers num(jd);
    t.start();
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        SkyPoint *pt = &skypoints[i];
        OldPrecession::precess(pt,&num);
        OldPointFunctions::nutate(pt,&num);
        // FIXME: Add aberration
        OldConversions::EquatorialToHorizontal(pt,&LST,&lat);
    }
    int time = t.elapsed();
    kDebug() << "Old method took" << time << "ms";

    // Test with newer CPU vector3d method
    QVector<J2000Coord> cpu_input(NUM_TEST_POINTS);
    QVector<HorizontalCoord> cpu_output(NUM_TEST_POINTS);
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        cpu_input[i] = Convert::sphToVect(dec,ra);
    }
    CoordConversion c = Convert::EqToHor(LST,lat)
                      * Convert::Nutate(jd)
                      * Convert::PrecessTo(jd);
    t.restart();
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        cpu_output[i] = c * cpu_input[i];
    }
    time = t.elapsed();
    kDebug() << "Plain CPU took" << time << "ms";

    // Now use OpenCL
    QVector<Vector4d> bufferdata(NUM_TEST_POINTS);
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        bufferdata[i] = Convert::sphToVect4(dec,ra);
    }
    KSClContext ctx;
    QVERIFY(ctx.create());
    KSClBuffer buf = ctx.createBuffer(KSClBuffer::J2000Buffer, bufferdata);
    t.restart();
    KSClBuffer buf2 = ctx.applyConversion(c, KSClBuffer::J2000Buffer, buf);
    time = t.elapsed();
    kDebug() << "OpenCL took" << time << "ms";

    QVERIFY(true);
}


QTEST_MAIN(TestClConvert)

#include "testclconvert.moc"
