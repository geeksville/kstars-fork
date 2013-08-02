/***************************************************************************
                   datacomposite.h  -  K Desktop Planetarium
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

#ifndef DATACOMPOSITE_H
#define DATACOMPOSITE_H

// Qt
#include <QtCore/QObject>

// Data Components
#include "datacomponent.h"

/**
 * @class DataComposite
 *
 * The DataComposite is the root of the DataComponent hierarchy.
 *
 * @author Henry de Valence
 */
class DataComposite : public QObject, public DataComponent 
{
    Q_OBJECT
public:
    /**
     * @short Construct the data hierarchy.
     *
     * @param reciever an object that should recieve progress messages.
     */
    explicit DataComposite(QObject *reciever);

    /**
     * @short Send progress message up the datacomponent tree.
     */
    virtual void sendProgressMessage(const QString &message) const override;

    DataComposite(const DataComposite&) = delete;
    DataComposite& operator= (const DataComposite&) = delete;
signals:
    void progressMessage(const QString &message) const;
};

#endif
