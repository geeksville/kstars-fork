/***************************************************************************
              testconvertcoord.h - Tests coordinate conversions
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

#ifndef CONVERTCOORD_H
#define CONVERTCOORD_H

#include <QtTest>

class TestConvertCoord : public QObject
{
    Q_OBJECT
private slots:
    void testSphericalToVector();
    void testVectorToSpherical();
    void testRotB1950ToGal();
    void testConvertEqToEcl();
    void testConvertEqToHor();
    void testPrecession();
    void testNutation();
};

#endif
