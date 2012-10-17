/***************************************************************************
                          sdss.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Sept 7 2012
    copyright            : (C) 2012 by Lukasz Jaskiewicz
    email                : lucas.jaskiewicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sdss.h"

#include "skypoint.h"
#include "skyobject.h"
#include "kstars/kstarsdata.h"

QUrl SDSS::getSDSSUrl(const SkyPoint *const point)
{
    // TODO: Remove code duplication -- we have the same stuff
    // implemented in ObservingList::setCurrentImage() etc. in
    // tools/observinglist.cpp; must try to de-duplicate as much as
    // possible.
    QString URLprefix("http://casjobs.sdss.org/ImgCutoutDR6/getjpeg.aspx?");
    QString URLsuffix("&scale=1.0&width=600&height=600&opt=GST&query=SR(10,20)");
    dms ra(0.0), dec(0.0);
    QString RAString, DecString;

    // ra and dec must be the coordinates at J2000.  If we clicked on an object, just use the object's ra0, dec0 coords
    // if we clicked on empty sky, we need to precess to J2000.
    // check if clicked point is a SkyObject
    const SkyObject *obj = static_cast<const SkyObject*>(point);
    if(obj) {
        ra  = obj->ra0();
        dec = obj->dec0();
    } else {
        SkyPoint tempPoint;
        // move present coords temporarily to ra0,dec0 (needed for precessToAnyEpoch)
        tempPoint.setRA0(point->ra());
        tempPoint.setDec0(point->dec());
        tempPoint.precessFromAnyEpoch(KStarsData::Instance()->ut().djd(), J2000);

        ra = tempPoint.ra();
        dec = tempPoint.dec();
    }

    RAString = RAString.sprintf("ra=%f", ra.Degrees());
    DecString = DecString.sprintf("&dec=%f", dec.Degrees());

    // concatenate all the segments into the kview command line:
    return QUrl(URLprefix + RAString + DecString + URLsuffix);
}

