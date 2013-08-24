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

// KDE
#include <KDebug>

// Local
#include "kscontextcl.h"

using KSEngine::EarthVelocity_Type;
using namespace Eigen;

bool KSBufferCL::setData(const Matrix4Xf &data) {
    if( data.cols() != m_size )
        return false;
    cl_int err;
    void *ptr = CAST_INTO_THE_VOID(data.data());
    err = m_queue.enqueueWriteBuffer(m_buf,
    /* Blocking read              */ true,
    /* Zero offset                */ 0,
    /* Size to copy               */ data.size() * sizeof(float),
    /* Pointer to data            */ ptr,
    /* Events to wait on          */ nullptr,
    /* Result event               */ nullptr);
    return (err == CL_SUCCESS);
}

bool KSBufferCL::setData(const Matrix4Xd &data) {
    if( data.cols() != m_size )
        return false;
    Matrix4Xf cldata = data.cast<float>();
    return setData(cldata);
}
bool KSBufferCL::setData(const Matrix3Xd &data) {
    if( data.cols() != m_size )
        return false;
    Matrix4Xf cldata(4,data.cols());
    cldata.block(0,0,3,data.cols()) = data.cast<float>();
    return setData(cldata);
}

KSBufferCL::KSBufferCL(const KSEngine::CoordType   t,
                       const int                   size,
                       const cl::Buffer           &buf,
                       const KSContextCL          *context,
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

Matrix4Xd KSBufferCL::data4() const
{
    Matrix4Xf mat(4,m_size);
    cl_int err = m_queue.enqueueReadBuffer(m_buf,
    /* Blocking read                    */ true,
    /* Read offset                      */ 0,
    /* Number of bytes to read          */ mat.size()*sizeof(float),
    /* Pointer to write to              */ mat.data());
    Q_ASSERT( err == CL_SUCCESS );
    return mat.cast<double>();
}

Matrix3Xd KSBufferCL::data() const
{
    Matrix4Xf mat(4,m_size);
    cl_int err = m_queue.enqueueReadBuffer(m_buf,
    /* Blocking read                    */ true,
    /* Read offset                      */ 0,
    /* Number of bytes to read          */ mat.size()*sizeof(float),
    /* Pointer to write to              */ mat.data());
    Q_ASSERT( err == CL_SUCCESS );
    Matrix3Xd small = mat.block(0,0,3,m_size).cast<double>();
    return small;
}

void KSBufferCL::applyConversion(const Matrix3d             &m,
                                 const KSEngine::CoordType   newtype,
                                       KSBuffer             *dest) const
{
    // Get CL buffer from dest
    KSBufferCL *other = dynamic_cast<KSBufferCL*>(dest->d);
    // We need to construct a 4x4 matrix in row-major order, since
    // we give its rows to the CL kernel as vectors.
    Matrix<float,4,4,RowMajor> big = Matrix4f::Identity();
    big.block(0,0,3,3) = m.cast<float>();
    Vector4f m1 = big.row(0); 
    Vector4f m2 = big.row(1);
    Vector4f m3 = big.row(2);
    cl_float4 *clm1 = reinterpret_cast<cl_float4*>(&m1);
    cl_float4 *clm2 = reinterpret_cast<cl_float4*>(&m2);
    cl_float4 *clm3 = reinterpret_cast<cl_float4*>(&m3);
    // Set kernel arguments and run the kernel
    auto kern = m_context->m_kernel_applyMatrix;
    kern.setArg(0,*clm1);
    kern.setArg(1,*clm2);
    kern.setArg(2,*clm3);
    kern.setArg(3,m_buf);
    kern.setArg(4,other->m_buf);
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
    other->m_type = newtype;
}

void KSBufferCL::applyConversion(const Matrix3d             &m,
                                 const KSEngine::CoordType   newtype)
{
    // We need to construct a 4x4 matrix in row-major order, since
    // we give its rows to the CL kernel as vectors.
    Matrix<float,4,4,RowMajor> big = Matrix4f::Identity();
    big.block(0,0,3,3) = m.cast<float>();
    Vector4f m1 = big.row(0); 
    Vector4f m2 = big.row(1);
    Vector4f m3 = big.row(2);
    cl_float4 *clm1 = reinterpret_cast<cl_float4*>(&m1);
    cl_float4 *clm2 = reinterpret_cast<cl_float4*>(&m2);
    cl_float4 *clm3 = reinterpret_cast<cl_float4*>(&m3);
    // Set kernel arguments and run the kernel
    auto kern = m_context->m_kernel_applyMatrixInPlace;
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
    if( m_type != EarthVelocity_Type )
        kFatal() << "Can't aberrate without changing coord systems!";
    auto kern = m_context->m_kernel_aberrate;
    kern.setArg(0,static_cast<float>(expRapidity));
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

KSBufferCL *KSBufferCL::clone() const
{
    // 1. Create a new buffer, the same size as the old one.
    cl_int err;
    // FIXME: maybe we should do some renaming?
    // m_context is a KSContext*; it has a cl::Context member m_context.
    cl::Buffer newBuf(m_context->m_context,
    /* Type flags  */ CL_MEM_READ_WRITE,
    /* # of bytes  */ m_size * 4 * sizeof(float),
    /* data ptr    */ nullptr,
    /* error ptr   */ &err);
    if( err != CL_SUCCESS )
        kFatal() << "Could not create buffer with error" << err;

    // 2. Copy the old buffer into the new one.
    cl::Event event;
    err = m_queue.enqueueCopyBuffer(m_buf,
    /* Dest buffer               */ newBuf,
    /* Source offset             */ 0,
    /* Dest offset               */ 0,
    /* # of bytes                */ m_size * 4 * sizeof(float),
    /* Event waitlist            */ nullptr,
    /* Wait event                */ &event);

    // 3. Wait for copy to finish.
    event.wait();
    if( err != CL_SUCCESS )
        kFatal() << "Could not copy buffer with error" << err;

    // 4. Create a new KSBufferCL object.
    return new KSBufferCL(m_type, m_size, newBuf, m_context, m_queue);
}
