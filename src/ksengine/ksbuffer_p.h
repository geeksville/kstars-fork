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

#ifndef KSBUFFER_P_H
#define KSBUFFER_P_H

#include "ksbuffer.h"

/* This is just some bullshit since CL C++ always takes void* */
template<typename T> 
void* CAST_INTO_THE_VOID(const T *t) { 
    return const_cast<void*>(static_cast<const void*>(t)); 
};

class KSBufferPrivate
{
public:
    /**
     * Try to set the data in this buffer from the data vector given.
     */
    virtual bool setData(const Eigen::Matrix3Xd &data) = 0;
    virtual Eigen::Matrix3Xd data() const = 0;
    virtual void applyConversion(const Eigen::Matrix3d      &m,
                                 const KSBuffer::BufferType  newtype) = 0;
    virtual void aberrate(const double expRapidity) = 0;
    virtual ~KSBufferPrivate() {};
    //virtual void copyFrom(const KSBuffer& other) = 0;
    KSBuffer::BufferType m_type;
    int m_size;
};

#endif

