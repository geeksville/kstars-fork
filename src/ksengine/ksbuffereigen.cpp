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

bool KSBufferEigen::setData(const Matrix3Xd &data) {
    if( data.cols() != m_size )
        return false;
    m_data = data;
    return true;
}

KSBufferEigen::KSBufferEigen(const KSEngine::CoordType   t,
                             const Eigen::Matrix3Xd     &data)
{
    m_type = t;
    m_size = data.cols();
    m_data = data;
}

KSBufferEigen::~KSBufferEigen()
{
}

Matrix3Xd KSBufferEigen::data() const
{
    return m_data;
}

void KSBufferEigen::applyConversion(const Matrix3d             &m,
                                    const KSEngine::CoordType   newtype)
{
    m_data = m*m_data;
    m_type = newtype;
}

void KSBufferEigen::aberrate(const double expRapidity)
{
    if( m_type != EarthVelocity_Type )
        kFatal() << "Can't aberrate without changing coord systems!";
    for(int i = 0; i < m_size; ++i) {
        m_data.col(i) = Convert::Aberrate(m_data.col(i), expRapidity);
    }
}

