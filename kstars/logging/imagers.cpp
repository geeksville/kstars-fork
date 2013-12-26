/***************************************************************************
                    imagers.cpp - K Desktop Planetarium
                             -------------------
    begin                : Tue Nov 12 2013
    copyright            : (C) 2013 by Rafal Kulaga
    email                : rl.kulaga@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "imagers.h"

using namespace Logging;

Imager::Imager(const int id, const QString &model, const QString &vendor, const QString &remarks) :
    m_Id(id), m_Model(model), m_Vendor(vendor), m_Remarks(remarks)
{ }



CcdCamera::CcdCamera(const int id, const QString &model, const QString &vendor, const QString &remarks,
                     const unsigned int pixelsX, const unsigned int pixelsY, const double pixelXSize,
                     const double pixelYSize, const unsigned int binning) :
    Imager(id, model, vendor, remarks), m_PixelsX(pixelsX), m_PixelsY(pixelsY), m_PixelXSize(pixelXSize),
    m_PixelYSize(pixelYSize), m_Binning(binning)
{ }



