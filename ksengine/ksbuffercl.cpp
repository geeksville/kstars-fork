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

#include "ksbuffercl.h"

// Eigen
using namespace Eigen;

// KDE
#include <KDebug>

// Local
#include "kscontext.h"
#include "kscontext_p.h"

bool KSBufferCL::setData(const Matrix4Xd &data) {
    if( data.cols() != m_size )
        return false;
    cl_int err;
    void *ptr = CAST_INTO_THE_VOID(data.data());
    err = m_queue.enqueueWriteBuffer(m_buf,
    /* Blocking read              */ true,
    /* Zero offset                */ 0,
    /* Size to copy               */ data.size() * sizeof(double),
    /* Pointer to data            */ ptr,
    /* Events to wait on          */ nullptr,
    /* Result event               */ nullptr);
    return (err == CL_SUCCESS);
}

KSBufferCL::KSBufferCL(const KSBuffer::BufferType  t,
                       const int                   size,
                       const cl::Buffer           &buf,
                       const KSContext            *context,
                       const cl::CommandQueue     &queue)
{
    m_type = t;
    m_size = size;
    m_context = context;
    m_queue = queue;
    m_buf = buf;
}

KSBufferCL::~KSBufferCL()
{
}

Matrix4Xd KSBufferCL::data() const
{
    Matrix4Xd mat(4,m_size);
    cl_int err = m_queue.enqueueReadBuffer(m_buf,
    /* Blocking read                    */ true,
    /* Read offset                      */ 0,
    /* Number of bytes to read          */ mat.size()*sizeof(double),
    /* Pointer to write to              */ mat.data());
    Q_ASSERT( err == CL_SUCCESS );
    return mat;
}

void KSBufferCL::applyConversion(const Matrix3d             &m,
                                 const KSBuffer::BufferType  newtype)
{
    // We need to construct a 4x4 matrix in row-major order, since
    // our CL kernel expects it that way.
    Matrix<double,4,4,RowMajor> big = Matrix4d::Identity();
    big.block(0,0,3,3) = m;
    Vector4d m1 = big.row(0); 
    Vector4d m2 = big.row(1);
    Vector4d m3 = big.row(2);
    cl_double4 *clm1 = reinterpret_cast<cl_double4*>(&m1);
    cl_double4 *clm2 = reinterpret_cast<cl_double4*>(&m2);
    cl_double4 *clm3 = reinterpret_cast<cl_double4*>(&m3);
    // Set kernel arguments and run the kernel
    auto kern = m_context->d->m_kernel_applyMatrix;
    kern.setArg(0,*clm1);
    kern.setArg(1,*clm2);
    kern.setArg(2,*clm3);
    kern.setArg(3,m_buf);
    cl::Event event;
    cl_int err = m_queue.enqueueNDRangeKernel(kern,
    /* Work range offset -- no offset      */ cl::NullRange,
    /* Global ID NDRange                   */ cl::NDRange(m_size),
    /* Local  ID NDRange                   */ cl::NDRange(1),
    /* Event waitlist                      */ nullptr,
    /* Output event                        */ &event);
    event.wait();
    if( err != CL_SUCCESS )
        kFatal() << "Failed executing kernel with error" << err;
    m_type = newtype;
}

void KSBufferCL::aberrate(const double expRapidity)
{
    if( m_type != KSBuffer::EarthVelocityBuffer )
        kFatal() << "Can't aberrate without changing coord systems!";
    auto kern = m_context->d->m_kernel_aberrate;
    kern.setArg(0,expRapidity);
    kern.setArg(1,m_buf);
    cl::Event event;
    cl_int err = m_queue.enqueueNDRangeKernel(kern,
    /* Work range offset -- no offset      */ cl::NullRange,
    /* Global ID NDRange                   */ cl::NDRange(m_size),
    /* Local  ID NDRange                   */ cl::NDRange(1),
    /* Event waitlist                      */ nullptr,
    /* Output event                        */ &event);
    event.wait();
    if( err != CL_SUCCESS )
        kFatal() << "Failed executing kernel with error" << err;
}

#if 0
void KSBufferCL::copyFrom(const KSBuffer& other)
{
    if( other.size() != this->size() )
        kFatal() << "Tried to copyFrom() buffers of different sizes!";
    cl::Event event;
    cl_int err = d->m_queue.enqueueCopyBuffer(other.d->m_buf,
    /* Destination buffer                  */ d->m_buf,
    /* Source offset                       */ 0,
    /* Destination offset                  */ 0,
    /* Num of bytes to copy                */ this->size()*sizeof(Vector4d),
    /* Waitlist                            */ nullptr,
    /* Completion event                    */ &event);
    event.wait();
    if( err != CL_SUCCESS ) {
        kFatal() << "Failed to copy buffer with error" << err;
    }
    d->m_type = other.d->m_type;
}
#endif

