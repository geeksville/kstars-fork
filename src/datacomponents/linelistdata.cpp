/***************************************************************************
                linelistdata.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 2013-08-01
    copyright            : (C) 2013 Henry de Valence
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

#include "linelistdata.h"

#include "ksengine/convertcoord.h"
#include "ksengine/dms.h"
#include "ksengine/ksbuffer.h"

using namespace KSEngine;

void LineListData::createEquator(DataComponent *parent)
{
    // We construct a LineListData object with an empty list
    LineListData *ll = new LineListData("Equator", parent);

    // Now, create a single buffer and add it to the list.
    static const int num_points = 360;
    Matrix3Xd data(3,num_points);
    QVector<int> indices(num_points+1);
    for(int i = 0; i < num_points; ++i) {
        // Point with dec = 0, ra = i*DEG2RAD
        data.col(i) = Convert::sphToVect( 0., i*DEG2RAD );
        indices[i] = i;
    }
    indices[num_points] = 0;

    KSContext *c = ll->context();
    KSBuffer *buf = new KSBuffer(c,Equatorial_Type,data);
    KSBuffer *bufCopy = new KSBuffer(*buf);
    ll->m_data = buf;
    ll->m_currentData = bufCopy;
    ll->m_lines.push_back(indices);

    // Finally, set the type.
    ll->m_type = Equatorial_Type;

    // No need to return ll as it's already been added to the tree.
}

LineListData::~LineListData()
{
    delete m_data;
    delete m_currentData;
}

void LineListData::update( const JulianDate  jd, 
                           const dms        &lat, 
                           const dms        &LST )
{

    CoordConversion conv;
    CoordType       newtype;
    switch( m_type ) {
        // Todo: add time-varying coordinate systems here.
        //case J2000_Type:
        default:
            //
            // In this case we assume that the coordinates do not
            // vary with time or location, so we just recurse and
            // return.
            //
            DataComponent::update( jd, lat, LST );
            return;
    }

    // Update data
    m_data->applyConversion(conv,newtype,m_currentData);
    // Update children
    DataComponent::update( jd, lat, LST );
}


LineListData::LineListData( const QString &id, DataComponent *parent )
    : DataComponent( id, parent )
{
    // Do nothing -- this is a private method, and the LLD object
    // is always made with one of our factory functions, which do
    // the initialization of private members.
}

