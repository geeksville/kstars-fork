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

#ifndef VARSTAROBSFINDINGS_H
#define VARSTAROBSFINDINGS_H

#include "optional.h"
#include "obsfindings.h"
#include "QStringList"

namespace Logging
{

class VarStarObsFindings : public ObsFindings
{
public:
    VarStarObsFindings(const int id, const QString &description, const QString &lang,
                       const double visMag, const QStringList &comparisonStars,
                       const QString &chartId);

    VarStarObsFindings(const int id, const QString &description, const QString &lang, const double visMag,
                       const bool visMagFainterThan, const bool visMagUncertain, const QStringList &comparisonStars,
                       const QString &chartId, const bool chartNonAAVSO, const bool brightSky, const bool clouds,
                       const bool poorSeeing, const bool nearHorizon, const bool unusualActivity, const bool outburst,
                       const bool comparismProblem, const bool starIdentUncertain, const bool faintStar);

    double visualMagnitude() const
    {
        return m_VisualMag;
    }

    Optional<bool> visualMagnitudeFainterThan() const
    {
        return m_VisMagFainterThan;
    }

    Optional<bool> visualMagnitudeUncertain() const
    {
        return m_VisMagUncertain;
    }

    QStringList comparisonStars() const
    {
        return m_ComparisonStars;
    }

    QString chartId() const
    {
        return m_ChartId;
    }

    Optional<bool> chartNonAAVSO() const
    {
        return m_ChartNonAAVSO;
    }

    Optional<bool> brightSky() const
    {
        return m_BrightSky;
    }

    Optional<bool> clouds() const
    {
        return m_Clouds;
    }

    Optional<bool> poorSeeing() const
    {
        return m_PoorSeeing;
    }

    Optional<bool> nearHorizon() const
    {
        return m_NearHorizon;
    }

    Optional<bool> unusualActivity() const
    {
        return m_UnusualActivity;
    }

    Optional<bool> outburst() const
    {
        return m_Outburst;
    }

    Optional<bool> comparismSeqProblem() const
    {
        return m_ComparismSeqProblem;
    }

    Optional<bool> starIdentUncertain() const
    {
        return m_StarIdentUncertain;
    }

    Optional<bool> faintStar() const
    {
        return m_FaintStar;
    }


    void setVisualMagnitude(const double visMag)
    {
        m_VisualMag = visMag;
    }

    void setVisualMagnitudeFainterThan(const Optional<bool> &fainterThan)
    {
        m_VisMagFainterThan = fainterThan;
    }

    void setVisualMagnitudeUncertain(const Optional<bool> &uncertain)
    {
        m_VisMagUncertain = uncertain;
    }

    void setComparisonStars(const QStringList &stars)
    {
        m_ComparisonStars = stars;
    }

    void setChartId(const QString &chartId)
    {
        m_ChartId = chartId;
    }

    void setChartNonAAVSO(const Optional<bool> &nonAAVSO)
    {
        m_ChartNonAAVSO = nonAAVSO;
    }

    void setBrightSky(const Optional<bool> &brightSky)
    {
        m_BrightSky = brightSky;
    }

    void setClouds(const Optional<bool> &clouds)
    {
        m_Clouds = clouds;
    }

    void setPoorSeeing(const Optional<bool> &poorSeeing)
    {
        m_PoorSeeing = poorSeeing;
    }

    void setNearHorizon(const Optional<bool> &nearHorizon)
    {
        m_NearHorizon = nearHorizon;
    }

    void setUnusualActivity(const Optional<bool> &unusualActivity)
    {
        m_UnusualActivity = unusualActivity;
    }

    void setOutburst(const Optional<bool> &outburst)
    {
        m_Outburst = outburst;
    }

    void setComparismSeqProblem(const Optional<bool> &seqProblem)
    {
        m_ComparismSeqProblem = seqProblem;
    }

    void setStarIdentUncertain(const Optional<bool> &uncertain)
    {
        m_StarIdentUncertain = uncertain;
    }

    void setFaintStar(const Optional<bool> &faintStar)
    {
        m_FaintStar = faintStar;
    }

private:
    double m_VisualMag;
    Optional<bool> m_VisMagFainterThan;
    Optional<bool> m_VisMagUncertain;

    QStringList m_ComparisonStars;
    QString m_ChartId;
    Optional<bool> m_ChartNonAAVSO;

    Optional<bool> m_BrightSky;
    Optional<bool> m_Clouds;
    Optional<bool> m_PoorSeeing;
    Optional<bool> m_NearHorizon;
    Optional<bool> m_UnusualActivity;
    Optional<bool> m_Outburst;
    Optional<bool> m_ComparismSeqProblem;
    Optional<bool> m_StarIdentUncertain;
    Optional<bool> m_FaintStar;
};

}

#endif // VARSTAROBSFINDINGS_H
