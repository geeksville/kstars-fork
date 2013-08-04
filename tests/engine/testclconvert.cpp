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
#include "ksengine/kscontext.h"
#include "ksengine/ksbuffer.h"
#include "ksengine/convertcoord.h"
#include "ksengine/astrovars.h"
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
    dms lat(43.7), LST(50.0);
    JulianDate jd = EpochJ2000 + 36525.0;

    dms *ra = new dms[NUM_TEST_POINTS];
    dms *dec = new dms[NUM_TEST_POINTS];
    for(int i = 0; i < 1024; ++i) {
        for(int j = 0; j < 1024; ++j) {
            ra[i*1024 + j] = dms(360.0*double(i)/1024);
            dec[i*1024 + j] = dms(-90.0 + 180*double(j)/1023);
        }
    }

    // Test with the old spherical trig method
    QVector<SkyPoint> skypoints(NUM_TEST_POINTS);
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        skypoints[i] = SkyPoint(ra[i],dec[i]);
    }
    KSNumbers num(jd);
    t.start();
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        SkyPoint *pt = &skypoints[i];
        OldPrecession::precess(pt,&num);
        OldPointFunctions::nutate(pt,&num);
        OldPointFunctions::aberrate(pt,&num);
        OldConversions::EquatorialToHorizontal(pt,&LST,&lat);
    }
    int time = t.elapsed();
    kDebug() << "Old method took" << time << "ms";

    // Test with KSBuffer

    // Get the coordinate conversion
    CoordConversion toEarthVel   = Convert::EclToEarthVel(jd)
                                 * Convert::EqToEcl(jd)
                                 * Convert::Nutate(jd)
                                 * Convert::PrecessTo(jd);
    
    CoordConversion fromEarthVel = Convert::EqToHor(LST,lat)
                                 * Convert::EclToEq(jd)
                                 * Convert::EarthVelToEcl(jd);
    
    double expRapidity = AstroVars::expRapidity(AstroVars::earthVelocity(jd));

    Matrix3Xd bufferdata(3,NUM_TEST_POINTS);
    for(int i = 0; i < NUM_TEST_POINTS; ++i) {
        bufferdata.col(i) = Convert::sphToVect(dec[i],ra[i]);
    }
    KSContext ctx;
    KSBuffer buf(&ctx, J2000_Type, bufferdata);
    t.restart();
        buf.applyConversion(toEarthVel, EarthVelocity_Type);
        buf.aberrate(expRapidity);
        buf.applyConversion(fromEarthVel, Horizontal_Type);
    time = t.elapsed();
    kDebug() << "KSBuffer took" << time << "ms";

    Matrix3Xd opencl_output = buf.data();
    Vector3d v_skyp = Convert::sphToVect(skypoints[0].alt(),
                                         skypoints[1].az());
    Vector3d v_cl   = opencl_output.col(0);

    QVERIFY(v_cl.isApprox(v_skyp));
}


QTEST_MAIN(TestClConvert)

#include "testclconvert.moc"
