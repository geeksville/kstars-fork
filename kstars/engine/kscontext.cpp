/***************************************************************************
   engine/kscontext.h - Class to manage contextual state for computation
                             -------------------
    begin                : 2013-07-06
    copyright            : (C) 2013 by Henry de Valence
    email                : hdevalence@hdevalence.ca
 ***************************************************************************/

/*
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

//This class
#include "kscontext.h"
#include "kscontext_p.h"

//OpenCL includes
#define __NO_STD_VECTOR // use cl vectors
#include <CL/cl.hpp>
#include <string>

//Qt includes
#include <QtCore/QString>

//KDE includes
#include <kdebug.h>

KSContextPrivate::KSContextPrivate(KSContext *q) : q(q)
{
    // We should set up an openCL context etc. here,
    // and this ctor should take an optional pointer to a GL 
    // context for CL/GL interop. But for now, we just query info.
    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    printf("boo\n");
    kDebug() << "Found " << platforms.size() << " platforms";
    for( auto platform : platforms ) {
        kDebug() << "Platform:";
        std::string s;
        platform.getInfo(CL_PLATFORM_VENDOR, &s);
        kDebug() << "CL_PLATFORM_VENDOR" << '\t' << QString::fromStdString(s);
        platform.getInfo(CL_PLATFORM_NAME, &s);
        kDebug() << "CL_PLATFORM_NAME" << '\t' << QString::fromStdString(s);
        platform.getInfo(CL_PLATFORM_VERSION, &s);
        kDebug() << "CL_PLATFORM_VERSION" << '\t' << QString::fromStdString(s);
        platform.getInfo(CL_PLATFORM_PROFILE, &s);
        kDebug() << "CL_PLATFORM_PROFILE" << '\t' << QString::fromStdString(s);
        platform.getInfo(CL_PLATFORM_EXTENSIONS, &s);
        kDebug() << "CL_PLATFORM_EXTENSIONS" << '\t' << QString::fromStdString(s);
    }
}

KSContextPrivate::~KSContextPrivate()
{
}

namespace KSEngine {

KSContext::KSContext()
    : d(new KSContextPrivate(this))
{
}

KSContext::~KSContext()
{
    delete d;
}

} // NS KSEngine

