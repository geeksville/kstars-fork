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

#include "ksbuffereigen.h"

// KDE
#include <KDebug>

// Local
#include "kscontext.h"
#include "kscontext_p.h"
#include "convertcoord.h"

using namespace KSEngine;
using namespace Eigen;

bool KSBufferEigen::setData(const Matrix4Xd &data) {
    if( data.cols() != m_size )
        return false;
    m_data = data;
    return true;
}

KSBufferEigen::KSBufferEigen(const KSBuffer::BufferType  t,
                             const Eigen::Matrix4Xd     &data)
{
    m_type = t;
    m_size = data.cols();
    m_data = data;
}

KSBufferEigen::~KSBufferEigen()
{
}

Matrix4Xd KSBufferEigen::data() const
{
    return m_data;
}

void KSBufferEigen::applyConversion(const Matrix3d             &m,
                                 const KSBuffer::BufferType  newtype)
{
    // We need to construct a 4x4 matrix in row-major order, since
    // our CL kernel expects it that way.
    Matrix<double,4,4,RowMajor> big = Matrix4d::Identity();
    big.block(0,0,3,3) = m;
    m_data *= big;
    m_type = newtype;
}

void KSBufferEigen::aberrate(const double expRapidity)
{
    if( m_type != KSBuffer::EarthVelocityBuffer )
        kFatal() << "Can't aberrate without changing coord systems!";
    for(int i = 0; i < m_size; ++i) {
        //FIXME: this duplicates code from KSEngine::Convert
        Vector4d p = m_data.col(i);
        if(p.y() < 0) {
            Vector2d ab = (expRapidity/(1-p.y()))*Vector2d(p.x(),p.z());
            double n = ab.squaredNorm();
            p = (1/(1+n))*Vector4d(2*ab(0), n-1, 2*ab(1), 0);
        } else {
            Vector2d ab = (1/(expRapidity*(1+p.y())))*Vector2d(p.x(),p.z());
            double n = ab.squaredNorm();
            p = (1/(1+n))*Vector4d(2*ab(0), -n+1, 2*ab(1), 0);
        }
        m_data.col(i) = p;
    }
}

