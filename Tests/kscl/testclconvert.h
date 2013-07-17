/***************************************************************************
 *         testclconvert.h - Tests conversion of points using KSCl
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

#ifndef TESTCLCONVERT_H
#define TESTCLCONVERT_H

#include <QtTest>

class TestClConvert : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testApparentCoord();
};

#endif 
