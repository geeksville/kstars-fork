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
    KSBuffer buf = c.createBuffer(KSBuffer::J2000Buffer, bufferdata);
    QVERIFY(true);
}

void TestKSContext::testApplyConversion()
{
    Matrix3Xd bufferdata(3,1024);
    for(int i = 0; i < 1024; ++i) {
        bufferdata.col(i) = Vector3d::UnitX();
    }
    KSContext c;
    KSBuffer buf = c.createBuffer(KSBuffer::J2000Buffer, bufferdata);
    Matrix3d m;
    m << 0, 1, 0,
         1, 0, 0,
         0, 0, 1;
    buf.applyConversion(m,KSBuffer::J2000Buffer);
    Matrix3Xd newdata = buf.data();
    bool ok = true;
    for( int i = 0; i < 1024; ++i) {
        ok &= (newdata.col(i) == Vector3d::UnitY());
    }
    QVERIFY(ok);
}


QTEST_MAIN(TestKSContext)

#include "testkscontext.moc"
