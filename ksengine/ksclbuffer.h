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

#ifndef KSCLBUFFER_H
#define KSCLBUFFER_H

#include <Eigen/Core>

namespace cl {
    class Buffer;
    class Context;
    class CommandQueue;
}

class KSClBufferPrivate;
class KSClContext;

class KSClBuffer
{
public:
    /** Describes the type of vector contained in the buffer. */
    enum BufferType {
        J2000Buffer,
        EquatorialBuffer,
        EclipticBuffer,
        HorizontalBuffer,
        EarthVelocityBuffer
    };
    friend class KSClContext;
    KSClBuffer(const KSClBuffer &other);
    ~KSClBuffer();
    KSClBuffer &operator=(const KSClBuffer &other);

    /**
     * @return the type of vector contained in this buffer.
     */
    BufferType type() const;

    /**
     * @return the number of elements in this buffer.
     */
    int size() const;

    /**
     * @return a matrix with the data for this buffer.
     * The columns of the matrix are the points of the buffer.
     */
    Eigen::Matrix4Xd data() const;

    /**
     * Applies the matrix @p m to the points in the buffer in-place
     * and changes the coordinate type to @p newtype.
     */
    void applyConversion(const Eigen::Matrix3d &m,
                         const BufferType       newtype);

    /**
     * Copies the coordinate data in @p other into this buffer
     * and also changes the type of this buffer to match @p other.
     */
    void copyFrom(const KSClBuffer& other);

    void aberrate(const double expRapidity);

private:
    KSClBuffer(const BufferType        t,
               const int               size,
               const cl::Buffer       &buf,
               const KSClContext      *context,
               const cl::CommandQueue &queue);
    KSClBufferPrivate *d;
};

#endif // KSCLBUFFER_H
