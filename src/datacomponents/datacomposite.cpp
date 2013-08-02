/***************************************************************************
                datacomposite.cpp  -  K Desktop Planetarium
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

#include "datacomposite.h"

using KSEngine::JulianDate;

DataComposite::DataComposite( QObject *reciever, KSContext *context ) 
    : DataComponent( "DataComposite", nullptr )
{
    // Set context pointer for tree BEFORE initialising other objects.
    this->m_context = context;
    // TODO: connect reciever
}

void DataComposite::sendProgressMessage( const QString &message ) const
{
    emit progressMessage(message);
}

#include "datacomposite.moc"
