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

#ifndef KSBUFFER_H
#define KSBUFFER_H

#include "kstypes.h"

class KSBufferPrivate;
class KSContext;

/**
 * @short A class to hold a buffer of points.
 *
 * KSBuffer holds an array of points and provides methods to 
 * apply common astronomical calculations to the points. For example,
 * it can compute the effects of aberration for every point in the
 * array. The nice thing about KSBuffer is that the implementation
 * details are hidden from the user. There are two backends; one 
 * uses OpenCL, while the other uses Eigen.
 *
 * Buffers are created either from a KSContext instance or from another
 * buffer. There is no implicit sharing, so buffers should either
 * be passed by const reference or using a pointer (if they are to be
 * modified).
 *
 * @author Henry de Valence
 */
class KSBuffer
{
public:
    // The KSContext backends need access to the internals
    // of the buffers that they are creating.
    friend class KSContextCL;
    friend class KSContextEigen;

    ~KSBuffer();
    /**
     * Construct a buffer
     * @param context a pointer to the parent context
     * @param t       the type of the coordinates in the buffer
     * @param data    a matrix whose columns are the points.
     */
    KSBuffer(      KSContext           *context,
             const KSEngine::CoordType  t,
             const Eigen::Matrix3Xd    &data);
    KSBuffer(const KSBuffer &other);
    KSBuffer &operator=(const KSBuffer &other);

    /**
     * @return the type of vector contained in this buffer.
     */
    KSEngine::CoordType type() const;

    /**
     * @return the number of elements in this buffer.
     */
    int size() const;

    /**
     * @return a matrix with the data for this buffer.
     * The columns of the matrix are the points of the buffer.
     */
    Eigen::Matrix3Xd data() const;

    /**
     * Applies the matrix @p m to the points in the buffer in-place
     * and changes the coordinate type to @p newtype.
     */
    void applyConversion(const Eigen::Matrix3d     &m,
                         const KSEngine::CoordType  newtype);

    /**
     * @short Perform aberration calculation on this buffer (in-place).
     * @param expRapidity @see AstroVars::expRapidity
     * @see Convert::Aberrate
     */
    void aberrate(const double expRapidity);

private:
    /**
     * Construct a buffer using the given d-pointer.
     * The buffer takes ownership of the pointer.
     *
     * We use this in the KSContext backends to construct KSBuffers
     * which have a particular backend implementation -- either 
     * using Eigen or OpenCL.
     */
    KSBuffer(KSBufferPrivate *dptr);
    KSBufferPrivate *d;
};

#endif // KSBUFFER_H
