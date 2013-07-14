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

namespace cl {
    class Buffer;
    class Event;
}

class KSClBufferPrivate;

class KSClBuffer
{
public:
    friend class KSClContext;
    KSClBuffer(const KSClBuffer &other);
    ~KSClBuffer();
    KSClBuffer &operator=(const KSClBuffer &other);
    /** Waits for any pending operations on the buffer to finish.
     *  For example, if this buffer is supposed to hold the output of
     *  some computation, then you need to call wait() to ensure that
     *  the computation is finished before using it.
     */
    void wait();
    /** Get the event for this buffer. */
    cl::Event event();
private:
    KSClBuffer(const cl::Buffer& buf);
    KSClBufferPrivate *d;
};

#endif // KSCLBUFFER_H
