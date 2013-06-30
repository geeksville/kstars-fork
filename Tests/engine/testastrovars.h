/***************************************************************************
 *         testastrovars.h - Tests calculation of misc. parameters.
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

#ifndef TESTASTROVARS_H
#define TESTASTROVARS_H

#include <QtTest>

class TestAstroVars : public QObject
{
    Q_OBJECT
private slots:
    void testNutationVars();
};

#endif 