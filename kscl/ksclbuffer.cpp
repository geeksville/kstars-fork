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

#include "ksclbuffer.h"
#include "ksclbuffer_p.h"

// OpenCL
#define __NO_STD_VECTOR
#include <CL/cl.hpp>

// Eigen
#include <Eigen/Core>
using namespace Eigen;

// KDE
#include <KDebug>

KSClBufferPrivate::KSClBufferPrivate(const cl::Buffer& buf)
{
    m_buf = buf;
}

bool KSClBufferPrivate::setData(const QVector<Vector4d>& data) {
    if( data.size() != m_size )
        return false;
    cl_int err;
    void *ptr = CAST_INTO_THE_VOID(data.data());
    err = m_queue.enqueueWriteBuffer(m_buf,
    /* Blocking read              */ true,
    /* Zero offset                */ 0,
    /* Size to copy               */ data.size() * sizeof(Vector4d),
    /* Pointer to data            */ ptr,
    /* Events to wait on          */ nullptr,
    /* Result event               */ nullptr);
    return (err == CL_SUCCESS);
}

KSClBuffer::KSClBuffer(const BufferType        t,
                       const int               size,
                       const cl::Buffer       &buf,
                       const cl::Context      &context,
                       const cl::CommandQueue &queue)
    : d(new KSClBufferPrivate(buf))
{
    d->m_type = t;
    d->m_size = size;
    d->m_context = context;
    d->m_queue = queue;
}

KSClBuffer::~KSClBuffer()
{
    delete d;
}

int KSClBuffer::size() const
{
    return d->m_size;
}

KSClBuffer::BufferType KSClBuffer::type() const
{
    return d->m_type;
}

QVector<Vector4d> KSClBuffer::data() const
{
    QVector<Vector4d> buf(this->size());
    cl_int err = d->m_queue.enqueueReadBuffer(d->m_buf,
    /* Blocking read                       */ true,
    /* Read offset                         */ 0,
    /* Number of bytes to read             */ this->size() * sizeof(Vector4d),
    /* Pointer to write to                 */ buf.data());
    Q_ASSERT( err == CL_SUCCESS );
    return buf;
}


