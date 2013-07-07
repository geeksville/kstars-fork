/***************************************************************************
 *         testkscontext.cpp - Tests the KSContext class.
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

#include "kstars/engine/kscontext.h"

using namespace KSEngine;

void TestKSContext::testCreation()
{
    KSContext *c = new KSContext();
    QCOMPARE(0,0);
}

QTEST_MAIN(TestKSContext)

#include "testkscontext.moc"
