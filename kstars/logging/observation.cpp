/***************************************************************************
                    observation.cpp - K Desktop Planetarium
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

#include "observation.h"

using namespace Logging;

Observation::Observation(const int id, const int observerId, const int sessionId, const int targetId,
                         const QDateTime &begin, const QDateTime &end, const double nakedEyeMagLim,
                         const double skyQuality, const int seeing, const int scopeId,
                         const QString &accessories, const int eyepieceId, const int lensId,
                         const int filterId, const double magnification, const int imagerId,
                         const QStringList &images, const QList<int> findingsIds) :
    m_Id(id), m_ObserverId(observerId), m_SessionId(sessionId), m_TargetId(targetId), m_Begin(begin),
    m_End(end), m_NakedEyeStarMagLimit(nakedEyeMagLim), m_SkyQuality(skyQuality), m_SeeingAntoniadi(seeing),
    m_ScopeId(scopeId), m_Accessories(accessories), m_EyepieceId(eyepieceId), m_LensId(lensId), m_FilterId(filterId),
    m_MagnificationUsed(magnification), m_ImagerId(imagerId), m_Images(images), m_FindingsIds(findingsIds)
{ }
