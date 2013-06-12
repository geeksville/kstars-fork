/***************************************************************************
  engine/oldprecession.cpp - deprecated functions for calculating precession.
                             -------------------
    begin                : 2013-06-12
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

#include "oldprecession.h"

#include <kdebug.h>

#include "skypoint.h"
#include "ksnumbers.h"

namespace KSEngine {
namespace OldPrecession {

SkyPoint deprecess(SkyPoint* p, const KSNumbers* num, const JulianDate epoch)
{
    SkyPoint p1( p->ra(), p->dec() );
    long double now = num->julianDay();
    p1.precessFromAnyEpoch( now, epoch );
    if( p->ra0().Degrees() < 0.0 || p->dec0().Degrees() > 90.0 ) 
    {
        kWarning() << "Invalid catalog coordinates in deprecess";
        // We have invalid RA0 and Dec0, so set them.
        p->setRA0(p1.ra());
        p->setDec0(p1.dec());
    }
    return p1;
}

} // NS OldPrecession
} // NS KSEngine