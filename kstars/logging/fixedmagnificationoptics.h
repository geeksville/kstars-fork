/***************************************************************************
                    fixedmagnificationoptics.h - K Desktop Planetarium
                             -------------------
    begin                : Tue Nov 12 2013
    copyright            : (C) 2013 by Rafal Kulaga
    email                : rl.kulaga@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FIXEDMAGNIFICATIONOPTICS_H
#define FIXEDMAGNIFICATIONOPTICS_H

#include "logging/optics.h"
#include "dms.h"

namespace Logging
{

class FixedMagnificationOptics : public Optics
{
public:
    FixedMagnificationOptics(const int id, const QString &model, const QString &type, const QString &vendor,
                             const double aperture, const double lightGrasp, const bool orientationErect,
                             const bool orientationTruesided, const double magnification, const dms &trueField);

    double magnification() const
    {
        return m_Magnification;
    }

    dms trueField() const
    {
        return m_TrueField;
    }

    void setMagnification(const double magnification)
    {
        m_Magnification = magnification;
    }

    void setTrueField(const dms &trueField)
    {
        m_TrueField = trueField;
    }

private:
    double m_Magnification;
    dms m_TrueField;
};

}

#endif // FIXEDMAGNIFICATIONOPTICS_H
