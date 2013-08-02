/***************************************************************************
                datacomponent.cpp  -  K Desktop Planetarium
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

#include "datacomponent.h"

using KSEngine::JulianDate;

DataComponent::DataComponent( const QString &id, DataComponent *parent ) 
    : m_parent( parent ),
      m_id( id )
{
    m_parent->addChild( this );
}

DataComponent::~DataComponent()
{
    for( auto child : m_children )
        delete child;
}

void DataComponent::update( JulianDate jd )
{
    for( auto child : m_children )
        child->update( jd );
}

QString DataComponent::id() const
{
    return m_id;
}

void DataComponent::sendProgressMessage( const QString &message ) const
{
    m_parent->sendProgressMessage( message );
}

DataComponent* DataComponent::parent() const
{
    return m_parent;
}

void DataComponent::addChild(DataComponent *child)
{
    m_children.push_back(child);
}

QVector<DataComponent*> DataComponent::children() const
{
    return m_children;
}


