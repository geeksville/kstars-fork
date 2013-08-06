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
class KSContext;
class dms;

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
 * Each component has a unique id; all the components in the heirarchy
 * share a common KSContext.
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

    /**
     * @return an internal id string unique to this component.
     */
    QString id() const;

    /**
     * @return a user-visible name for this component. Purely cosmetic.
     */
    QString name() const;

    KSContext* context() const;

    /**
     * @short Perform a DFS for the DataComponent with the given @p id.
     * @return that component if found, otherwise nullptr.
     * @note Behaviour is undefined if multiple components
     *       have the same id.
     */
    DataComponent* findById(const QString &id);

protected:

    /**
     * @short Constructor
     * @p id a unique name for this component.
     * @p name a user-visible name for the component.
     * @p parent pointer to the parent DataComponent.
     * @note The parent must not be null.
     */
    explicit DataComponent( const QString       &id,
                            const QString       &name,
                                  DataComponent *parent );
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
     * must have computed the correct coordinates
     * for the given Julian date and location. The datacomponent
     * may choose what coordinate system to use (e.g., horizontal,
     * equatorial, ecliptic, etc.), but the coordinates must be valid
     * for the given date and location.
     *
     * For instance, a data component holding stars would need to
     * compute precession, nutation, aberration, proper motion, etc.,
     * to obtain coordinates valid for the given Julian date.
     * It is then the responsibility of the user of the component to
     * perform the conversion from the coordinate system used by
     * the component.
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
     * @param lat the latitude of the observer
     * @param LST the local sidereal time of the observer
     * @note Do we need the geographic location here?
     */
    virtual void update( const KSEngine::JulianDate  jd,
                         const dms                  &lat,
                         const dms                  &LST );

    /**
     * @short Send progress message up the datacomponent tree.
     *
     * This is used to show a loading screen on startup.
     */
    virtual void sendProgressMessage( const QString &message ) const;

    void addChild(DataComponent* child);

    DataComponent(const DataComponent&) = delete;
    DataComponent& operator= (const DataComponent&) = delete;

    // Normally, the m_context pointer is obtained from the parent.
    // However, DataComposite is the root, so it needs to set it,
    // but we don't want to have a way to set the context otherwise:
    // it should be the same for the whole tree. So we just give
    // access to DataComposite.
    friend class DataComposite;
private:
    DataComponent           *m_parent;
    KSContext               *m_context;
    QVector<DataComponent*>  m_children;
    QString                  m_id;
    QString                  m_name;
};

#endif
