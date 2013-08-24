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

#ifndef KSBUFFEREIGEN_H
#define KSBUFFEREIGEN_H

// Local
#include "ksbuffer_p.h"

class KSBufferEigen : public KSBufferPrivate
{
public:
    friend class KSContextEigen;
    virtual ~KSBufferEigen();

    /**
     * Return a deep copy of this buffer.
     */
    virtual KSBufferEigen* clone() const override;

    virtual Eigen::Matrix3Xd data() const override;
    virtual Eigen::Matrix4Xd data4() const override;

    virtual bool setData(const Eigen::Matrix4Xd &data)
                 override;
    virtual bool setData(const Eigen::Matrix3Xd &data)
                 override;

    virtual void applyConversion(const Eigen::Matrix3d      &m,
                                 const KSEngine::CoordType   newtype) 
                 override;
    virtual void applyConversion(const Eigen::Matrix3d      &m,
                                 const KSEngine::CoordType   newtype,
                                       KSBuffer             *dest)
                 const override;

    virtual void aberrate(const double expRapidity)
                 override;

private:
    /// Constructs a buffer of Quaternion_Type
    KSBufferEigen(const Eigen::Matrix4Xd     &data);
    /// Constructs a buffer of type @p t
    KSBufferEigen(const KSEngine::CoordType  t,
                  const Eigen::Matrix3Xd     &data);
    Eigen::Matrix4Xd m_data;
};

#endif // KSBUFFEREIGEN_H
