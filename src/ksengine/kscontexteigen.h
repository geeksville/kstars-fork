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

#ifndef KSCONTEXTEIGEN_H
#define KSCONTEXTEIGEN_H

// Eigen
#include <Eigen/Core>

#include "kscontext_p.h"
#include "ksbuffer.h"

class KSContextEigen : public KSContextPrivate
{
public:
    friend class KSBufferEigen;
    KSContextEigen();
    virtual ~KSContextEigen();

    /**
     * @return true if we were able to successfully create a context.
     */
    virtual bool isValid() override;

    /**
     * Create a buffer that lives in OpenCL.
     * @param t the type of coordinates that are in this buffer.
     * @param data a 4xN matrix whose columns are the points.
     */
    KSBuffer createBuffer(const KSEngine::CoordType     t,
                          const Eigen::Matrix3Xd       &data);

    //Disallow copy and assignment
    KSContextEigen &operator=(const KSContextEigen &other) = delete;
    KSContextEigen(const KSContextEigen &other) = delete;
};

#endif // KSCONTEXTEIGEN_H
