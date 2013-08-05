/***************************************************************************
                   linelistdata.h  -  K Desktop Planetarium
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

#ifndef LINELISTDATA_H
#define LINELISTDATA_H

// Qt
#include <QtCore/QVector>
#include <QtCore/QList>

// Data Components
#include "datacomponent.h"

class KSBuffer;

/**
 * @class LineListData
 *
 * LineListData holds a list of lines.
 *
 * @author Henry de Valence
 */
class LineListData : public DataComponent 
{
public:
    /**
     * @short creates a LineListData holding the equator.
     */
    static void createEquator(DataComponent *parent);
    virtual ~LineListData();

    LineListData(const LineListData&) = delete;
    LineListData& operator= (const LineListData&) = delete;

    virtual void update( const KSEngine::JulianDate  jd,
                         const dms                  &lat,
                         const dms                  &LST ) override;
private:
    LineListData(const QString &id, DataComponent *parent);
    KSEngine::CoordType m_type;
    /**
     * The m_data array holds all of the points that make up the lines
     * of the LineListData.
     */
    const KSBuffer *m_data;
    /// m_currentData contains the current position of the lines.
    KSBuffer *m_currentData;
    /**
     * The lines themselves are stored in vectors of index positions.
     * For instance, {12,3,5} is a line running from m_data[12]
     * to m_data[3] to m_data[5].
     */
    QList<QVector<int> > m_lines;
};

#endif
