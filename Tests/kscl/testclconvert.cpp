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
#include <KDebug>

// Local
#include "ksengine/ksclcontext.h"
#include "ksengine/ksclbuffer.h"
#include "ksengine/convertcoord.h"
#include "kstars/oldengine/oldconversions.h"
#include "kstars/oldengine/oldprecession.h"
#include "kstars/oldengine/oldpointfunctions.h"
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

    // Get the coordinate conversion
    CoordConversion c = Convert::EqToHor(LST,lat)
                      * Convert::Nutate(jd)
                      * Convert::PrecessTo(jd);
    
    // Test with newer CPU vector3d method
    Matrix3Xd cpu_input3(3,NUM_TEST_POINTS);
    Matrix3Xd cpu_output3(3,NUM_TEST_POINTS);
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        cpu_input3.col(i) = Convert::sphToVect(dec,ra);
    }
    t.restart();
    cpu_output3 = c * cpu_input3;
    time = t.elapsed();
    kDebug() << "Plain CPU took" << time << "ms";

    // Now use OpenCL
    Matrix4Xd bufferdata(4,NUM_TEST_POINTS);
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        bufferdata.col(i) = Convert::sphToVect4(dec,ra);
    }
    KSClContext ctx;
    QVERIFY(ctx.create());
    KSClBuffer buf = ctx.createBuffer(KSClBuffer::J2000Buffer, bufferdata);
    t.restart();
    buf.applyConversion(c, KSClBuffer::HorizontalBuffer);
    time = t.elapsed();
    kDebug() << "OpenCL took" << time << "ms";

    Matrix4Xd opencl_output = buf.data();
    Vector3d v_skyp = Convert::sphToVect(skypoints[0].alt(),
                                         skypoints[1].az());
    Vector3d v_cpu  = cpu_output3.col(0);
    Vector3d v_cl   = opencl_output.block(0,0,3,1);

    QVERIFY(v_skyp.isApprox(v_cpu));
    QVERIFY(v_cpu.isApprox(v_cl));
    QVERIFY(v_cl.isApprox(v_skyp));
}


QTEST_MAIN(TestClConvert)

#include "testclconvert.moc"
