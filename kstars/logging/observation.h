/***************************************************************************
                    observation.h - K Desktop Planetarium
                             -------------------
    begin                : Mon Dec 2 2013
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

#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "optional.h"
#include "QString"
#include "QStringList"
#include "QDateTime"

namespace Logging
{

class Observation
{
public:
    Observation(const int id, const int observerId, const int targetId, const QDateTime &begin);

    Observation(const int id, const int observerId, const int sessionId, const int targetId,
                const QDateTime &begin, const QDateTime &end, const double nakedEyeMagLim,
                const double skyQuality, const int seeing, const int scopeId,
                const QString &accessories, const int eyepieceId, const int lensId,
                const int filterId, const double magnification, const int imagerId,
                const QStringList &images, const QList<int> findingsIds);

    int id() const
    {
        return m_Id;
    }

    int observerId() const
    {
        return m_ObserverId;
    }

    Optional<int> sessionId() const
    {
        return m_SessionId;
    }

    int targetId() const
    {
        return m_TargetId;
    }

    QDateTime begin() const
    {
        return m_Begin;
    }

    Optional<QDateTime> end() const
    {
        return m_End;
    }

    Optional<double> nakedEyeStarMagLimit() const
    {
        return m_NakedEyeStarMagLimit;
    }

    Optional<double> skyQuality() const
    {
        return m_SkyQuality;
    }

    Optional<int> seeingAntoniadi() const
    {
        return m_SeeingAntoniadi;
    }

    Optional<int> scopeId() const
    {
        return m_ScopeId;
    }

    QString accessories() const
    {
        return m_Accessories;
    }

    Optional<int> eyepieceId() const
    {
        return m_EyepieceId;
    }

    Optional<int> lensId() const
    {
        return m_LensId;
    }

    Optional<int> filterId() const
    {
        return m_FilterId;
    }

    Optional<double> magnificationUsed() const
    {
        return m_MagnificationUsed;
    }

    Optional<int> imagerId() const
    {
        return m_ImagerId;
    }

    QStringList images() const
    {
        return m_Images;
    }

    QList<int> findingsIds() const
    {
        return m_FindingsIds;
    }

    void setObserverId(const int observerId)
    {
        m_ObserverId = observerId;
    }

    void setSessionId(const Optional<int> &sessionId)
    {
        m_SessionId = sessionId;
    }

    void setTargetId(const int targetId)
    {
        m_TargetId = targetId;
    }

    void setBegin(const QDateTime &begin)
    {
        m_Begin = begin;
    }

    void setEnd(const Optional<QDateTime> &end)
    {
        m_End = end;
    }

    void setNakedEyeStarMagLimit(const Optional<double> &magLimit)
    {
        m_NakedEyeStarMagLimit = magLimit;
    }

    void setSkyQuality(const Optional<double> &quality)
    {
        m_SkyQuality = quality;
    }

    void setSeeingAntoniadi(const Optional<int> &seeing)
    {
        m_SeeingAntoniadi = seeing;
    }

    void setScopeId(const Optional<int> &scopeId)
    {
        m_ScopeId = scopeId;
    }

    void setAccessories(const QString &accessories)
    {
        m_Accessories = accessories;
    }

    void setEyepieceId(const Optional<int> &eyepieceId)
    {
        m_EyepieceId = eyepieceId;
    }

    void setLensId(const Optional<int> &lensId)
    {
        m_LensId = lensId;
    }

    void setFilterId(const Optional<int> &filterId)
    {
        m_FilterId = filterId;
    }

    void setMagnificationUsed(const Optional<double> &magnification)
    {
        m_MagnificationUsed = magnification;
    }

    void setImagerId(const Optional<int> &imagerId)
    {
        m_ImagerId = imagerId;
    }

    void setImages(const QStringList &images)
    {
        m_Images = images;
    }

    void setFindingsIds(const QList<int> findingsIds)
    {
        m_FindingsIds = findingsIds;
    }

private:
    int m_Id;
    int m_ObserverId;
    Optional<int> m_SessionId;
    int m_TargetId;
    QDateTime m_Begin;
    Optional<QDateTime> m_End;
    Optional<double> m_NakedEyeStarMagLimit;
    Optional<double> m_SkyQuality;
    Optional<int> m_SeeingAntoniadi;
    Optional<int> m_ScopeId;
    QString m_Accessories;
    Optional<int> m_EyepieceId;
    Optional<int> m_LensId;
    Optional<int> m_FilterId;
    Optional<double> m_MagnificationUsed;
    Optional<int> m_ImagerId;
    QStringList m_Images;
    QList<int> m_FindingsIds;
};

}

#endif // OBSERVATION_H
