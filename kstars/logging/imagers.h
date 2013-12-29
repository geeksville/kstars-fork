/***************************************************************************
                    imagers.h - K Desktop Planetarium
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

#ifndef IMAGERS_H
#define IMAGERS_H

#include "optional.h"
#include "QString"

namespace Logging
{

class Imager
{
public:
    Imager(const int id, const QString &model);

    Imager(const int id, const QString &model, const QString &vendor, const QString &remarks);

    int id() const
    {
        return m_Id;
    }

    QString model() const
    {
        return m_Model;
    }

    QString vendor() const
    {
        return m_Vendor;
    }

    QString remarks() const
    {
        return m_Remarks;
    }

    void setModel(const QString &model)
    {
        m_Model = model;
    }

    void setVendor(const QString &vendor)
    {
        m_Vendor = vendor;
    }

    void setRemarks(const QString &remarks)
    {
        m_Remarks = remarks;
    }

private:
    int m_Id;
    QString m_Model;
    QString m_Vendor;
    QString m_Remarks;
};


class CcdCamera : public Imager
{
public:
    CcdCamera(const int id, const QString &model, const unsigned int pixelsX,
              const unsigned int pixelsY, const unsigned int binning = 1);

    CcdCamera(const int id, const QString &model, const QString &vendor, const QString &remarks,
              const unsigned int pixelsX, const unsigned int pixelsY, const double pixelXSize,
              const double pixelYSize, const unsigned int binning);

    unsigned int pixelsX() const
    {
        return m_PixelsX;
    }

    unsigned int pixelsY() const
    {
        return m_PixelsY;
    }

    Optional<double> pixelXSize() const
    {
        return m_PixelXSize;
    }

    Optional<double> pixelYSize() const
    {
        return m_PixelYSize;
    }

    unsigned int binning() const
    {
        return m_Binning;
    }

    void setPixelsX(const unsigned int pixelsX)
    {
        m_PixelsX = pixelsX;
    }

    void setPixelsY(const unsigned int pixelsY)
    {
        m_PixelsY = pixelsY;
    }

    void setPixelXSize(const Optional<double> pixelXSize)
    {
        m_PixelXSize = pixelXSize;
    }

    void setPixelYSize(const Optional<double> pixelYSize)
    {
        m_PixelYSize = pixelYSize;
    }

    void setBinning(const unsigned int binning)
    {
        m_Binning = binning;
    }


private:
    unsigned int m_PixelsX;
    unsigned int m_PixelsY;
    Optional<double> m_PixelXSize;
    Optional<double> m_PixelYSize;
    unsigned int m_Binning;
};

}

#endif // IMAGERS_H
