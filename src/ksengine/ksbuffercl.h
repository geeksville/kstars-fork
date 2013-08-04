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

/**
 * @short OpenCL backend for KSBuffer
 *
 * Notes:
 *
 * - Currently, we use floats instead of doubles for all the actual
 *   computation with OpenCL, since GPUs don't have very fast 
 *   double-precision arithmetic.
 *
 * - Currently, everything is done totally synchronously. It would 
 *   probably be better to have the KSBuffer implementation actually use
 *   the OpenCL event mechanism. This way, for instance, we can
 *   return immediately from one of the conversion functions, and then
 *   wait for the computation to finish when we actually want to use the
 *   results. Hopefully, they've actually finished the computation by then.
 *
 */
class KSBufferCL : public KSBufferPrivate
{
public:
    friend class KSContextCL;
    virtual ~KSBufferCL();
    virtual KSBufferCL* clone() const override;

    virtual Eigen::Matrix3Xd data() const override;
    virtual bool setData(const Eigen::Matrix3Xd &data) override;

    virtual void applyConversion(const Eigen::Matrix3d      &m,
                                 const KSEngine::CoordType   newtype) 
                 override;

    virtual void aberrate(const double expRapidity) override;

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
