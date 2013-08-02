/***************************************************************************
                   datacomponent.h  -  K Desktop Planetarium
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

#ifndef DATACOMPONENT_H
#define DATACOMPONENT_H

// Qt
#include <QtCore/QVector>
#include <QtCore/QString>

// KSEngine
#include "src/ksengine/kstypes.h"

/**
 * @class DataComponent
 *
 * A DataComponent is a data structure that holds some object or
 * objects which should be rendered on the sky map. It is responsible
 * for loading the data and keeping it up-to-date as needed, but not
 * for drawing the data.
 *
 * The data components are organized as a tree structure; each
 * component has a parent and zero or more children.
 *
 * @author Henry de Valence
 */
class DataComponent
{
public:

    /**
     * @return This component's parent.
     * @note This is null if and only if this component is the 
     *       root component.
     */
    DataComponent* parent() const;

    QVector<DataComponent*> children() const;

    QString id() const;

protected:

    /**
     * @short Constructor
     * @p id a unique name for this component.
     * @p parent pointer to the parent DataComponent.
     * @note The parent must not be null.
     */
    explicit DataComponent( const QString       &id,
                                  DataComponent *parent );

    virtual ~DataComponent();

    /**
     * @short Update this component and its children.
     *
     * The contract for this function is: after it is called,
     * this component and all of its child components
     * must have computed the Equatorial coordinates
     * for the given Julian date. 
     *
     * Since the implementation for this class obeys the contract,
     * i.e., it just calls update() on its children,
     * inheritors of this class can implement the function as
     * follows:
     *
     *     void SomeData::update( JulianDate jd )
     *     {
     *         // update this component
     *         doStuff();
     *         // update children
     *         DataComponent::update(jd);
     *     }
     *
     * @param jd the current Julian date
     */
    virtual void update( KSEngine::JulianDate jd );

    /**
     * @short Send progress message up the datacomponent tree.
     *
     * This is used to show a loading screen on startup.
     */
    virtual void sendProgressMessage( const QString &message ) const;

    void addChild(DataComponent* child);

    DataComponent(const DataComponent&) = delete;
    DataComponent& operator= (const DataComponent&) = delete;
private:
    DataComponent           *m_parent;
    QVector<DataComponent*>  m_children;
    QString                  m_id;
};

#endif
