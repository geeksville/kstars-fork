/***************************************************************************
 *         testkscontext.h - Tests the KSContext class
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

#ifndef TESTKSCLCONTEXT_H
#define TESTKSCLCONTEXT_H

#include <QtTest>

class TestKSContext : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testCloneBuffer();
    void testCreateBuffer();
    void testApplyConversion();
};

#endif 
