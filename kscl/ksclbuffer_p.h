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

#ifndef KSCLBUFFER_P_H
#define KSCLBUFFER_P_H

#include "ksclbuffer.h"

// OpenCL
#define __NO_STD_VECTOR
#include <CL/cl.hpp>

// Eigen
#include <Eigen/Core>

/* This is just some bullshit since CL C++ always takes void* */
template<typename T> 
void* CAST_INTO_THE_VOID(const T *t) { 
    return const_cast<void*>(static_cast<const void*>(t)); 
};

class KSClContext;

class KSClBufferPrivate
{
public:
    KSClBufferPrivate(const cl::Buffer& buf);
    /**
     * Try to set the data in this buffer from the data vector given.
     */
    bool setData(const Eigen::Matrix4Xd &data);
    cl::Buffer m_buf;
    cl::CommandQueue m_queue;
    const KSClContext *m_context;
    KSClBuffer::BufferType m_type;
    int m_size;
};

#endif

