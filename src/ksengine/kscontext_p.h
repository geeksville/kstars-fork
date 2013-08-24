/*
 * KStars OpenCL Bindings
 * copyright    : (C) 2013 Henry de Valence
 * email        : hdevalence@hdevalence.ca
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KSCONTEXT_P_H
#define KSCONTEXT_P_H

#include "kscontext.h"

class KSContextPrivate {
public:
    virtual ~KSContextPrivate() {};
    /**
     * @return true if the context was successfully constructed.
     */
    virtual bool isValid() = 0;
    /**
     * Create a buffer implementation with the correct backend.
     * @param t the type of coordinates that are in this buffer.
     * @param data a 3xN matrix whose columns are the points.
     */
    virtual KSBufferPrivate* createBuffer(const KSEngine::CoordType   t,
                                          const Eigen::Matrix3Xd     &data) =0;
    /**
     * Create a buffer implementation with the correct backend.
     * @param data a 4xN matrix whose columns are quaternions.
     */
    virtual KSBufferPrivate* createBuffer(const Eigen::Matrix4Xd     &data) =0;
};

#endif

