/***************************************************************************
                    varstarobsfindings.h - K Desktop Planetarium
                             -------------------
    begin                : Sun Dec 8 2013
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

#include "varstarobsfindings.h"

using namespace Logging;

VarStarObsFindings::VarStarObsFindings(const int id, const QString &description, const QString &lang,
                                       const double visMag, const QStringList &comparisonStars,
                                       const QString &chartId) :
    ObsFindings(id, description, lang), m_VisualMag(visMag), m_ComparisonStars(comparisonStars),
    m_ChartId(chartId)
{ }

VarStarObsFindings::VarStarObsFindings(const int id, const QString &description, const QString &lang, const double visMag,
                                       const bool visMagFainterThan, const bool visMagUncertain, const QStringList &comparisonStars,
                                       const QString &chartId, const bool chartNonAAVSO, const bool brightSky, const bool clouds,
                                       const bool poorSeeing, const bool nearHorizon, const bool unusualActivity, const bool outburst,
                                       const bool comparismProblem, const bool starIdentUncertain, const bool faintStar) :
    ObsFindings(id, description, lang), m_VisualMag(visMag), m_VisMagFainterThan(visMagFainterThan), m_VisMagUncertain(visMagUncertain),
    m_ComparisonStars(comparisonStars), m_ChartId(chartId), m_ChartNonAAVSO(chartNonAAVSO), m_BrightSky(brightSky), m_Clouds(clouds),
    m_PoorSeeing(poorSeeing), m_NearHorizon(nearHorizon), m_UnusualActivity(unusualActivity), m_Outburst(outburst),
    m_ComparismSeqProblem(comparismProblem), m_StarIdentUncertain(starIdentUncertain), m_FaintStar(faintStar)
{ }
