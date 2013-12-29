/***************************************************************************
                    scope.h - K Desktop Planetarium
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

#ifndef SCOPE_H
#define SCOPE_H

#include "logging/optics.h"

namespace Logging
{

class Scope : public Optics
{
public:
    Scope(const int id, const QString &model, const double aperture, const double focalLength);

    Scope(const int id, const QString &model, const QString &type, const QString &vendor,
          const double aperture, const double lightGrasp, const bool orientationErect,
          const bool orientationTruesided, const double focalLength);

    double focalLength() const
    {
        return m_FocalLength;
    }

    void setFocalLength(const double focalLength)
    {
        m_FocalLength = focalLength;
    }

private:
    double m_FocalLength;
};

}
#endif // SCOPE_H
