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

#ifndef KSBUFFERCL_H
#define KSBUFFERCL_H

// OpenCL
#define __NO_STD_VECTOR
#include <CL/cl.hpp>

// Local
#include "ksbuffer_p.h"

class KSContextCL;

class KSBufferCL : public KSBufferPrivate
{
public:
    friend class KSContextCL;
    KSBufferCL(const KSBufferCL &other);
    virtual ~KSBufferCL();
    KSBufferCL &operator=(const KSBufferCL &other);

    virtual Eigen::Matrix3Xd data()
                             const override;
    virtual bool setData(const Eigen::Matrix3Xd &data)
                 override;

    virtual void applyConversion(const Eigen::Matrix3d      &m,
                                 const KSEngine::CoordType   newtype) 
                 override;

    virtual void aberrate(const double expRapidity)
                 override;

private:
    KSBufferCL(const KSEngine::CoordType   t,
               const int                   size,
               const cl::Buffer           &buf,
               const KSContextCL          *context,
               const cl::CommandQueue     &queue);
    const KSContextCL *m_context;
    cl::CommandQueue   m_queue;
    cl::Buffer         m_buf;
};

#endif // KSBUFFERCL_H
