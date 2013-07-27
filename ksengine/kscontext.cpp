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
#include "ksbuffer.h"
#include "ksbuffer_p.h"

using namespace Eigen;

KSContext::KSContext()
    : d(new KSContextPrivate)
{
    d->m_Valid = false;
}

KSContext::~KSContext()
{
    delete d;
}

bool KSContext::isValid()
{
    return d->m_Valid;
}

KSBuffer KSContext::createBuffer(const KSBuffer::BufferType  t,
                                     const Eigen::Matrix4Xd       &data)
{
    cl_int err;
    void *bufdata = CAST_INTO_THE_VOID(data.data());
    cl::Buffer clbuf(d->m_context,
    /* Type flags */ CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
    /* # of bytes */ data.size() * sizeof(double), 
    /* data ptr   */ bufdata, 
    /* error ptr  */ &err);
    if( err != CL_SUCCESS )
        kFatal() << "Could not create buffer with error" << err;
    return KSBuffer(t, data.cols(), clbuf, this, d->m_queue);
}

bool KSContext::create()
{
    //
    // Pick a device and platform to use.
    // We have some preferences, so we filter and sort the candidates,
    // and then take the best one.
    //

    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    typedef std::pair<cl::Platform, cl::Device> Candidate;

    std::vector<Candidate> candidates;
    for( auto platform : platforms ) {
        cl::vector<cl::Device> devices;
        platform.getDevices( CL_DEVICE_TYPE_ALL, &devices );
        for( auto device : devices ) {
            candidates.push_back(std::make_pair(platform, device));
        }
    }

    // Eliminate candidates that don't support FP64
    auto it = std::remove_if( candidates.begin(), candidates.end(), 
                              [](Candidate c) {
                                  std::string s; 
                                  c.second.getInfo( CL_DEVICE_EXTENSIONS, &s);
                                  return (s.find("cl_khr_fp64") == std::string::npos);
                              });
    candidates.resize(std::distance(candidates.begin(),it));

    // Prefer GPU devices
    auto isGPU = [](Candidate c) {
        cl_device_type t; c.second.getInfo(CL_DEVICE_TYPE, &t);
        return t == CL_DEVICE_TYPE_GPU;
    };
    std::stable_sort( candidates.begin(), candidates.end(), 
                      [&isGPU](Candidate a, Candidate b) {
                          // a < b when a is a GPU but b is not.
                          return isGPU(a) && !isGPU(b);
                      });

    // Prefer devices with GL interop
    auto hasGL = [](Candidate c) {
        std::string exts; c.second.getInfo(CL_DEVICE_EXTENSIONS, &exts);
        return (exts.find("cl_khr_gl_sharing") != std::string::npos);
    };
    std::stable_sort( candidates.begin(), candidates.end(), 
                      [&hasGL](Candidate a, Candidate b) {
                          // a < b when a has GL but b does not.
                          return hasGL(a) && !hasGL(b);
                      });

    if( candidates.empty() ) {
        kFatal() << "Could not find any candidate platforms!";
        return false;
    }
    Candidate c = candidates.front();
    std::string device_name, plat_name, plat_version;
    c.first.getInfo( CL_PLATFORM_NAME, &plat_name);
    c.first.getInfo( CL_PLATFORM_VERSION, &plat_version);
    c.second.getInfo( CL_DEVICE_NAME, &device_name);
    kDebug() << "Decided to try to use " << QString::fromStdString(device_name) 
             << "on" << QString::fromStdString(plat_name)
             << "version" << QString::fromStdString(plat_version);

    //
    // Now create the context.
    //
    
    cl_int err;
    /**
     * The properties array is an array of pairs name, value, 
     * terminated by null. Note that the OpenCL C++ API uses operator() 
     * to get the underlying OpenCL C handle.
     */
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(c.first)()
      , 0 };
    cl::vector<cl::Device> devices;
    devices.push_back(c.second);
    d->m_context = cl::Context(devices, properties, nullptr, nullptr, &err);
    d->m_device = devices.front();
    d->m_queue = cl::CommandQueue(d->m_context, d->m_device);

    if( err != CL_SUCCESS ) {
        kFatal() << "Could not create OpenCL context, error" << err;
        return false;
    } else {
        kDebug() << "Context created successfully";
    }

    //
    // Now load the kernels from a file
    //

    QString filename = KStandardDirs::locate("appdata","scripts/kernels.cl");
    kDebug() << "Trying to load kernels from" << filename;

    QFile sourcefile(filename);
    if( !sourcefile.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        kFatal() << "Can't open the kernel sources";
        return false;
    }
    QByteArray source = QTextStream(&sourcefile).readAll().toLocal8Bit();

    cl::Program::Sources sources(1,std::make_pair(source.data(),source.size()+1));
    d->m_program = cl::Program(d->m_context,sources,&err);
    if( err != CL_SUCCESS ) {
        kFatal() << "Dying with error" << err << "in cl::Program";
        return false;
    }
    err = d->m_program.build( devices, "-cl-std=CL1.1" );
    if( err != CL_SUCCESS ) {
        kFatal() << "Failed to compile the kernel!";
        return false;
    }

    d->m_kernel_applyMatrix = cl::Kernel(d->m_program,"applyMatrix",&err);
    if( err != CL_SUCCESS ) {
        kFatal() << "Failed to get a reference to __kernel applyMatrix";
        return false;
    }
    d->m_kernel_aberrate = cl::Kernel(d->m_program,"aberrate",&err);
    if( err != CL_SUCCESS ) {
        kFatal() << "Failed to get a reference to __kernel aberrate";
        return false;
    }

    d->m_Valid = true;
    return true;
}

