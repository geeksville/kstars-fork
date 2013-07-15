/***************************************************************************
 *         testksclcontext.cpp - tests KSClContext class
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

#include "testksclcontext.h"

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

void TestKSClContext::initTestCase()
{
    // We need to modify the directories, since we aren't running
    // as KStars itself, so it won't search the right places.
    KGlobal::dirs()->addResourceType("appdata","data","kstars");
}

void TestKSClContext::testCreation()
{
    KSClContext *c = new KSClContext();
    c->create();
    QVERIFY(c->isValid());
}

void TestKSClContext::testCreateBuffer()
{
    QVector<Vector4d> bufferdata(1024, Vector4d::UnitX());
    KSClContext c;
    c.create();
    KSClBuffer buf = c.createBuffer(KSClBuffer::J2000Buffer, bufferdata);
    QVERIFY(true);
}

QTEST_MAIN(TestKSClContext)

#include "testksclcontext.moc"
