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

DataComponent::DataComponent( const QString       &id,
                              const QString       &name,
                                    DataComponent *parent ) 
    : m_parent( parent )
    , m_id( id )
    , m_name( name )
{
    if( m_parent ) {
        m_parent->addChild( this );
        m_context = m_parent->context();
    }
    m_name = "";
}


DataComponent::DataComponent( const QString &id, DataComponent *parent ) 
    : m_parent( parent )
    , m_id( id )
    , m_name( "" )
{
    if( m_parent ) {
        m_parent->addChild( this );
        m_context = m_parent->context();
    }
}

DataComponent::~DataComponent()
{
    for( auto child : m_children )
        delete child;
}

DataComponent* DataComponent::findById(const QString &id)
{
    if( m_id == id )
        return this;
    // Search children by id, and return if found
    for( auto child : m_children ) {
        auto result = child->findById(id);
        if( result )
            return result;
    }
    // Not found in subtree, return nullptr
    return nullptr;
}

void DataComponent::update( const JulianDate  jd,
                            const dms        &lat,
                            const dms        &LST )
{
    for( auto child : m_children )
        child->update( jd, lat, LST );
}

QString DataComponent::id() const
{
    return m_id;
}

QString DataComponent::name() const
{
    return m_name;
}

KSContext* DataComponent::context() const
{
    return m_context;
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

