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

/**
 * @short An abstract class that hides the implementation of @class KSBuffer.
 *
 * We want to be able to use either Eigen on the CPU or OpenCL on 
 * the GPU, without forcing the rest of the program to know about this 
 * difference. Since C++ does polymorphism using pointers, this would mean
 * having all of the users of the KSBuffer class use a KSBuffer* and 
 * deal with pointers at all times. Instead, we make the d-pointer
 * @class KSBufferPrivate have the implementation details.
 *
 * The KSBufferPrivate class itself is abstract; it is inherited by
 * @class KSBufferEigen and @class KSBufferCL. 
 *
 * KSBuffer objects are created either by a @class KSContext, which has a
 * completely similar d-pointer structure, or by copy/assignment operations.
 * The choice of backend for the KSContext (either @class KSContextCL
 * or @class KSContextEigen) determines the class used as the d-pointer.
 *
 * @author Henry de Valence
 */
class KSBufferPrivate
{
public:
    /**
     * Try to set the data in this buffer from the data vector given.
     */
    virtual bool setData(const Eigen::Matrix3Xd &data) = 0;
    virtual bool setData(const Eigen::Matrix4Xd &data) = 0;
    virtual Eigen::Matrix3Xd data() const = 0;
    virtual Eigen::Matrix4Xd data4() const = 0;
    virtual void applyConversion(const Eigen::Matrix3d      &m,
                                 const KSEngine::CoordType   newtype) = 0;
    virtual void applyConversion(const Eigen::Matrix3d      &m,
                                 const KSEngine::CoordType   newtype,
                                       KSBuffer             *dest) const = 0;
    virtual void aberrate(const double expRapidity) = 0;
    virtual ~KSBufferPrivate() {};

    /**
     * @short Used to implement copy ctors and assignment.
     * 
     * Performs a deep copy of the buffer. We can't use copy constructors
     * here, since there's no way to have a "virtual copy constructor"
     * in C++.
     *
     * See 
     *  http://www.parashift.com/c++-faq/virtual-ctors.html
     * and also in particular
     *  http://yosefk.com/c++fqa/inheritance-virtual.html#fqa-20.8
     * for an explanation.
     */
    virtual KSBufferPrivate* clone() const = 0;

    /**
     * The type of coordinates that this buffer is holding.
     */
    KSEngine::CoordType m_type;
    /**
     * The number of elements in the buffer.
     */
    int m_size;
};

#endif

