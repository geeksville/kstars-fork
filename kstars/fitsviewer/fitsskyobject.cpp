/*
    fitsskyobject.cpp  -  FITS Image
    -------------------
    begin                : Tue Apr 07 2020
    SPDX-FileCopyrightText: 2004 Jasem Mutlaq (C) 2020 by Eric Dejouhanet <mutlaqja@ikarustech.com>

    SPDX-License-Identifier: GPL-2.0-or-later

    Some code fragments were adapted from Peter Kirchgessner's FITS plugin
    See http://members.aol.com/pkirchg for more details.
*/

#include "fitsskyobject.h"

FITSSkyObject::FITSSkyObject(SkyObject /*const*/ * object, int xPos, int yPos) : QObject()
{
    skyObjectStored = object;
    xLoc            = xPos;
    yLoc            = yPos;
}

SkyObject /*const*/ * FITSSkyObject::skyObject()
{
    return skyObjectStored;
}

int FITSSkyObject::x() const
{
    return xLoc;
}

int FITSSkyObject::y() const
{
    return yLoc;
}

void FITSSkyObject::setX(int xPos)
{
    xLoc = xPos;
}

void FITSSkyObject::setY(int yPos)
{
    yLoc = yPos;
}
