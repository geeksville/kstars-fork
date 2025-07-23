/*
    SPDX-FileCopyrightText: 2022 Toni Schriber

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/******************************************************************************************************
* Angle calculations are based on position measurements of
* - Rotator angle in "Circular Angle (A)" mode (0 <> 359.99° CCW)
* - Camera offset angle & Camera position angle in "Position Angle (PA)" mode (180 <> -179.99° CCW)
* This leads to the following calculations:
* - Camera PA = calcCameraAngel(Rotator A)
* - Rotator A = calcRotatorAngle(Camera PA)
* - Camera offset PA = calcOffsetAngle(Rotator A, Camera PA)
*******************************************************************************************************/

#include "rotatorutils.h"
#include "Options.h"

#include "opticaltrainmanager.h"

#include <indicom.h>
#include <basedevice.h>
#include <cmath>
#include "kstarsdata.h"

RotatorUtils * RotatorUtils::m_Instance = nullptr;

RotatorUtils * RotatorUtils::Instance()
{
    if (m_Instance)
        return m_Instance;

    m_Instance = new RotatorUtils();
    return m_Instance;
}

void RotatorUtils::release()
{
    delete (m_Instance);
    m_Instance = nullptr;
}

RotatorUtils::RotatorUtils() {}

RotatorUtils::~RotatorUtils() {}

void RotatorUtils::initRotatorUtils(const QString &train)
{
    m_Offset = Options::pAOffset();
    m_Mount = Ekos::OpticalTrainManager::Instance()->getMount(train);
    m_Camera = Ekos::OpticalTrainManager::Instance()->getCamera(train);
    KStarsData::Instance()->geo()->lat()->SinCos(m_SinLatitude, m_CosLatitude);
    m_TanLatitude = m_SinLatitude / m_CosLatitude;
    if (m_Mount)
    {
        connect(m_Mount, &ISD::Mount::pierSideChanged, this, [this] (ISD::Mount::PierSide Side)
        {
            m_flippedMount = (Side != m_CalPierside);
            emit changedPierside(Side);
        });
        connect(m_Mount, &ISD::Mount::newCoords, this, [this] (const SkyPoint &Position,
                                                               ISD::Mount::PierSide PierSide,
                                                               const dms &HourAngle)
        {
            Q_UNUSED(PierSide);   
            if (Options::useAltAz() && Options::astrometryUseDerotation())
            {
                m_Position = Position;
                m_HourAngle = HourAngle;

                double PARA = calcParallacticAngle(Position, HourAngle);
                double DeltaPARA = PARA - m_BaseParallacticAngle;

                if (fabs(DeltaPARA) > Options::astrometryDerotationThreshold()/60)
                {
                    emit adjustParallacticAngle(-(DeltaPARA), m_BaseParallacticAngle);
                    // setAltAZTrackrate(Position);
                    setBaseParallacticAngle();
                }
            }
        });
    }
}

/*void RotatorUtils::setAltAZTrackrate(const SkyPoint &Position)
{
    double sinAz, cosAz = 0;
    double sinAlt, cosAlt = 0;
    Position.az().SinCos(sinAz, cosAz);
    Position.alt().SinCos(sinAlt, cosAlt);
    m_Mount->setCustomTrackRate(15.041067 * (m_SinLatitude - ((cosAz * sinAlt * m_CosLatitude) / cosAlt)),
                                15.041067 * sinAz * m_CosLatitude);
}*/

void RotatorUtils::setBaseParallacticAngle()
{
    m_BaseParallacticAngle = calcParallacticAngle(m_Position, m_HourAngle);
}

double RotatorUtils::getBaseParallacticAngle()
{
    return m_BaseParallacticAngle;
}

double RotatorUtils::calcParallacticAngle(const SkyPoint &Position, const dms &HourAngle)
{
    double sinDEC, cosDEC = 0;
    double sinHA, cosHA = 0;
    m_Mount->blockSignals(true);
    HourAngle.SinCos(sinHA, cosHA);
    Position.dec().SinCos(sinDEC, cosDEC);
    m_Mount->blockSignals(false);
    double PARA = std::atan2(sinHA, (m_TanLatitude * cosDEC - sinDEC * cosHA)) / dms::DegToRad;  // in degrees
    return PARA;
}

double RotatorUtils::calcDerotationThreshold()
{
    double MaxAngle = -1;
    double CameraPixelWidth { -1 };
    double CameraPixelHeight { -1 };
    uint16_t CameraWidth { 0 };
    uint16_t CameraHeight { 0 };
    uint8_t bit_depth = 8;
    auto targetChip = m_Camera->getChip(ISD::CameraChip::PRIMARY_CCD);
    if (targetChip)
    {
        targetChip->getImageInfo(CameraWidth, CameraHeight, CameraPixelWidth, CameraPixelHeight, bit_depth);
        MaxAngle = 3.33 / ((sqrt((CameraWidth * CameraWidth) + (CameraHeight * CameraHeight))) * dms::DegToRad) * 60;
    }
    return MaxAngle;
}

double RotatorUtils::calcRotatorAngle(double PositionAngle)
{
    if (m_flippedMount)
    {
        PositionAngle += 180;
    }
    return KSUtils::range360(PositionAngle - m_Offset);
}

double RotatorUtils::calcCameraAngle(double RotatorAngle, bool flippedImage)
{
    double PositionAngle = 0;
    if (RotatorAngle > 180)
    {
        PositionAngle = (RotatorAngle - 360) + m_Offset;
    }
    else
    {
        PositionAngle = RotatorAngle + m_Offset;
    }
    if (!m_flippedMount != !flippedImage) // XOR
    {
        if (PositionAngle > 0)
        {
            PositionAngle -= 180;
        }
        else
        {
            PositionAngle += 180;
        }

    }
    return KSUtils::rangePA(PositionAngle);
}

double RotatorUtils::calcOffsetAngle(double RotatorAngle, double PositionAngle)
{
    double OffsetAngle = 0;
    if (RotatorAngle > 180)
    {
        OffsetAngle = PositionAngle - (RotatorAngle - 360);
    }
    else
    {
        OffsetAngle = PositionAngle - RotatorAngle;
    }
    if (m_flippedMount)
    {
        OffsetAngle -= 180;
    }
    return KSUtils::rangePA(OffsetAngle);
}

void RotatorUtils::updateOffset(double Angle)
{
    m_Offset = Angle;
    Options::setPAOffset(Angle);
}

ISD::Mount::PierSide RotatorUtils::getMountPierside()
{
    return(m_Mount->pierSide());
}

void RotatorUtils::setImagePierside(ISD::Mount::PierSide ImgPierside)
{
    m_ImgPierside = ImgPierside;
}

bool RotatorUtils::checkImageFlip()
{
    bool flipped = false;

    if (m_ImgPierside != ISD::Mount::PIER_UNKNOWN)
        if (!m_flippedMount != (m_ImgPierside == m_CalPierside)) // XOR
            flipped = true;
    return flipped;
}

double RotatorUtils::DiffPA(double diff)
{
    if (diff > 180)
        return (360 - diff);
    else
        return diff;
}

void RotatorUtils::initTimeFrame(const double EndAngle)
{
    m_EndAngle = EndAngle;
    m_initParameter = true;
    m_CCW = true;
}

int RotatorUtils::calcTimeFrame(const double CurrentAngle)
{
    m_CurrentTime = QTime::currentTime();
    m_DeltaTime = m_StartTime.secsTo(m_CurrentTime);
    m_TimeFrame = 0;
    if (m_DeltaTime >= 1)
    {
        if (m_initParameter)
        {
            m_DeltaAngle = CurrentAngle + m_ShiftAngle;
            // Moving CCW or positive
            if (m_DeltaAngle >= 360)
            {
                if (m_DiffAngle < 0)
                    m_DiffAngle = (360 + m_DiffAngle);
            }
            else // Moving CW or negative
            {
                if (m_DiffAngle > 0)
                    m_DiffAngle = (360 - m_DiffAngle);
                m_CCW = false;
            }
            m_initParameter = false;
        }
        m_DeltaAngle = KSUtils::range360(CurrentAngle + m_ShiftAngle);
        if (!m_CCW)
                m_DeltaAngle = 360 - m_DeltaAngle;

        m_TimeFrame = fabs(m_DiffAngle) / fabs(m_DeltaAngle/m_DeltaTime);
    }
    return m_TimeFrame;
}

void RotatorUtils::startTimeFrame(const double StartAngle)
{
    m_StartAngle = StartAngle;
    m_StartTime = QTime::currentTime();
    m_ShiftAngle = 360 - m_StartAngle;
    m_DiffAngle = m_EndAngle - m_StartAngle;
}
