/***************************************************************************
 *         testastrovars.cpp - Tests calculation of misc. parameters.
 *                             -------------------
 *    begin                : 2013-07-01
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

#include "testastrovars.h"

#include "ksengine/astrovars.h"
#include "kstars/ksnumbers.h"

using namespace KSEngine;

void TestAstroVars::testNutationVars()
{
    JulianDate jd = EpochJ2000 + 36525.;
    double avEcL, avOb;
    AstroVars::nutationVars(jd, &avEcL, &avOb);
    KSNumbers num(jd);
    printf("%f %f\n%f %f\n", avEcL, avOb, num.dEcLong(), num.dObliq());
    QCOMPARE( avEcL, num.dEcLong() );
    QCOMPARE( avOb, num.dObliq() );
}

QTEST_MAIN(TestAstroVars)

#include "testastrovars.moc"
