/***************************************************************************
              solarsystemlistcomponent.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 2005/22/09
    copyright            : (C) 2005 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "solarsystemlistcomponent.h"
#include "solarsystemcomposite.h"

#include <QPen>
#include <klocale.h>

#include "Options.h"
#include "skyobjects/ksplanet.h"
#include "skyobjects/ksplanetbase.h"
#include "kstarsdata.h"
#include "skymap.h"

#include "oldengine/oldconversions.h"
using namespace KSEngine;

SolarSystemListComponent::SolarSystemListComponent( SolarSystemComposite *p ) :
    ListComponent( p ),
    m_Earth( p->earth() )
{}

SolarSystemListComponent::~SolarSystemListComponent()
{
    //Object deletes handled by parent class (ListComponent)
}

void SolarSystemListComponent::update(KSNumbers * ) {
    if ( selected() ) {
        KStarsData *data = KStarsData::Instance(); 
        foreach ( SkyObject *o, m_ObjectList ) {
            // FIXME: get rid of cast. 
            KSPlanetBase *p = (KSPlanetBase*)o;
            OldConversions::EquatorialToHorizontal( p, data->lst(), data->geo()->lat() );
        }
    }
}

void SolarSystemListComponent::updatePlanets(KSNumbers *num ) {
    if ( selected() ) {
        KStarsData *data = KStarsData::Instance(); 
        foreach ( SkyObject *o, m_ObjectList ) {
            KSPlanetBase *p = (KSPlanetBase*)o;
            p->findPosition( num, data->geo()->lat(), data->lst(), m_Earth );
            OldConversions::EquatorialToHorizontal( p, data->lst(), data->geo()->lat() );

            if ( p->hasTrail() )
                p->updateTrail( data->lst(), data->geo()->lat() );
        }
    }
}


void SolarSystemListComponent::drawTrails( SkyPainter *skyp ) {
    //FIXME: here for all objects trails are drawn this could be source of inefficiency
    if( selected() )
        foreach( SkyObject *obj, m_ObjectList )
            // Will segfault if not TrailObject
            dynamic_cast<TrailObject*>(obj)->drawTrail(skyp);
}
