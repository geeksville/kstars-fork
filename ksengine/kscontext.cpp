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

#include "kscontext.h"
#include "kscontext_p.h"

// OPENCL
#define __NO_STD_VECTOR // use cl vectors
#include <CL/cl.hpp>

// C++
#include <string>

// Qt
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QByteArray>

// KDE
#include <KDebug>
#include <KStandardDirs>

// Local
#include "ksbuffercl.h"
#include "kscontextcl.h"

using namespace Eigen;

KSContext::KSContext()
{
    KSContextCL *d_cl = new KSContextCL();
    if(d_cl->isValid()) {
        d = d_cl;
    } else {
        delete d_cl;
        //FIXME: implement Eigen backend
        d = nullptr;
    }
}

KSContext::~KSContext()
{
    delete d;
}

KSBuffer KSContext::createBuffer(const KSBuffer::BufferType  t,
                                 const Eigen::Matrix4Xd     &data)
{
    return d->createBuffer(t,data);
}

