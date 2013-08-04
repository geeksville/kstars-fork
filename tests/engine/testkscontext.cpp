/***************************************************************************
 *         testkscontext.cpp - tests KSContext class
 *                             -------------------
 *    begin                : 2013-07-06
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

#include "testkscontext.h"

// Eigen
#include <Eigen/Core>
using namespace Eigen;

// KDE
#include <KStandardDirs>
#include <KGlobal>

// Local
#include "ksengine/kscontext.h"
#include "ksengine/ksbuffer.h"

using KSEngine::J2000_Type;

void TestKSContext::initTestCase()
{
    // We need to modify the directories, since we aren't running
    // as KStars itself, so it won't search the right places.
    KGlobal::dirs()->addResourceType("appdata","data","kstars");
}

void TestKSContext::testCreateBuffer()
{
    Matrix3Xd bufferdata(3,1024);
    for(int i = 0; i < 1024; ++i) {
        bufferdata.col(i) = Vector3d::UnitX();
    }
    KSContext c;
    KSBuffer buf(&c, J2000_Type, bufferdata);
    QVERIFY(true);
}

void TestKSContext::testApplyMatrix()
{
    /*
     * We make two buffers, one with X's and another
     * with Y's. Then we convert one to the other.
     * We are very loose with the checking because 
     * isApprox uses some matrix norm that's not really
     * relevant here, and we just want to make sure things
     * are sane in this test.
     */
    // Input/Output data and transformation
    Matrix3Xd xs(3,1024);
    Matrix3Xd ys(3,1024);
    Matrix3Xd newdata(3,1024);
    for(int i = 0; i < 1024; ++i) {
        xs.col(i) = Vector3d::UnitX();
        ys.col(i) = Vector3d::UnitY();
    }
    Matrix3d m;
    m << 0, 1, 0,
         1, 0, 0,
         0, 0, 1;

    // Create context and buffers
    KSContext c;
    const KSBuffer bufA(&c, J2000_Type, ys);
          KSBuffer bufB(&c, J2000_Type, xs);

    // 1. Convert bufB in-place
    bufB.applyConversion(m,J2000_Type);
    newdata = bufB.data();
    QVERIFY(newdata.isApprox(ys,1e-3));

    // 2. Convert bufA into bufB
    bufA.applyConversion(m,J2000_Type,&bufB);
    newdata = bufB.data();
    QVERIFY(newdata.isApprox(xs,1e-3));
}

void TestKSContext::testCloneBuffer()
{
    Matrix3Xd bufferdata(3,1024);
    for(int i = 0; i < 1024; ++i) {
        bufferdata.col(i) = Vector3d::UnitX();
    }
    KSContext c;
    KSBuffer buf1(&c, J2000_Type, bufferdata);
    KSBuffer buf2 = buf1;
    Matrix3Xd data2 = buf2.data();
    QVERIFY(data2.isApprox(bufferdata, 1e-7));
}

QTEST_MAIN(TestKSContext)

#include "testkscontext.moc"
