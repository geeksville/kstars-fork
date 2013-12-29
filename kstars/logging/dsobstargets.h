/***************************************************************************
                    dsobstargets.h - K Desktop Planetarium
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

#ifndef DSOBSTARGETS_H
#define DSOBSTARGETS_H

#include "optional.h"
#include "obstarget.h"

namespace Logging
{

class DsObsTarget : public ObsTarget
{
public:
    DsObsTarget(const int id, const QString &name);

    DsObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                const double surfBrightness);

    Optional<dms> smallDiameter() const
    {
        return m_SmallDiameter;
    }

    Optional<dms> largeDiameter() const
    {
        return m_LargeDiameter;
    }

    Optional<double> visualMagnitude() const
    {
        return m_VisualMagnitude;
    }

    Optional<double> surfaceBrightness() const
    {
        return m_SurfaceBrightness;
    }

    void setSmallDiameter(const Optional<dms> &smallDiameter)
    {
        m_SmallDiameter = smallDiameter;
    }

    void setLargeDiameter(const Optional<dms> &largeDiameter)
    {
        m_LargeDiameter = largeDiameter;
    }

    void setVisualMagnitude(const Optional<double> visualMagnitude)
    {
        m_VisualMagnitude = visualMagnitude;
    }

    void setSurfaceBrightness(const Optional<double> surfaceBrightness)
    {
        m_SurfaceBrightness = surfaceBrightness;
    }

private:
    Optional<dms> m_SmallDiameter;
    Optional<dms> m_LargeDiameter;
    Optional<double> m_VisualMagnitude;
    Optional<double> m_SurfaceBrightness;
};


class DsMultipleStarObsTarget : public ObsTarget
{
public:
    DsMultipleStarObsTarget(const int id, const QString &name, const QList<int> &componentsTargetIds);

    DsMultipleStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                            const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin, const REF_FRAME_EQUINOX equinox,
                            const QString &constellation, const QString &notes, const QList<int> &componentsTargetIds);

    QList<int> componentsTargetId() const
    {
        return m_ComponentsTargetIds;
    }

    void setComponentsTargetIds(const QList<int> &componentsTargetIds)
    {
        m_ComponentsTargetIds = componentsTargetIds;
    }

private:
    QList<int> m_ComponentsTargetIds;
};


class DsAsterismObsTarget : public DsObsTarget
{
public:
    DsAsterismObsTarget(const int id, const QString &name);

    DsAsterismObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                        const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                        const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                        const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                        const double surfBrightness, const double posAngle);

    Optional<double> positionAngle() const
    {
        return m_PositionAngle;
    }

    void setPositionAngle(const Optional<double> &posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    Optional<double> m_PositionAngle;
};


class DsGalaxyClusterObsTarget : public DsObsTarget
{
public:
    DsGalaxyClusterObsTarget(const int id, const QString &name);

    DsGalaxyClusterObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                             const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                             const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                             const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                             const double surfBrightness, const double brightest10thMag);

    Optional<double> brightest10thMag() const
    {
        return m_Brightest10thMag;
    }

    void setBrightest10thMag(const Optional<double> &mag)
    {
        m_Brightest10thMag = mag;
    }

private:
    Optional<double> m_Brightest10thMag;
};


class DsDarkNebulaObsTarget : public DsObsTarget
{
public:
    DsDarkNebulaObsTarget(const int id, const QString &name);

    DsDarkNebulaObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                           const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                           const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                           const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                           const double surfBrightness, const double posAngle, const int opacity);

    Optional<double> positionAngle() const
    {
        return m_PositionAngle;
    }

    Optional<int> opacity() const
    {
        return m_Opacity;
    }

    void setPositionAngle(const Optional<double> &posAngle)
    {
        m_PositionAngle = posAngle;
    }

    void setOpacity(const Optional<int> &opacity)
    {
        m_Opacity = opacity;
    }

private:
    Optional<double> m_PositionAngle;
    Optional<int> m_Opacity;
};


class DsDoubleStarObsTarget : public DsObsTarget
{
public:
    DsDoubleStarObsTarget(const int id, const QString &name);

    DsDoubleStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                          const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                          const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                          const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                          const double surfBrightness, const dms &separation, const double posAngle,
                          const double companionMag);

    Optional<dms> separation() const
    {
        return m_Separation;
    }

    Optional<double> positionAngle() const
    {
        return m_PositionAngle;
    }

    Optional<double> companionMagnitude() const
    {
        return m_CompanionMagnitude;
    }

    void setSeparation(const Optional<dms> &separation)
    {
        m_Separation = separation;
    }

    void setPositionAngle(const Optional<double> &posAngle)
    {
        m_PositionAngle = posAngle;
    }

    void setCompanionMagnitude(const Optional<double> &companionMag)
    {
        m_CompanionMagnitude = companionMag;
    }

private:
    Optional<dms> m_Separation;
    Optional<double> m_PositionAngle;
    Optional<double> m_CompanionMagnitude;
};


class DsGlobularClusterObsTarget : public DsObsTarget
{
public:
    DsGlobularClusterObsTarget(const int id, const QString &name);

    DsGlobularClusterObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                      const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                      const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                      const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                      const double surfBrightness, const double brightestMag, const QString &concentrationDeg);

    Optional<double> brightestStarMag() const
    {
        return m_BrightestStarMag;
    }

    QString concentrationDeg() const
    {
        return m_ConcentrationDeg;
    }

    void setBrightestStarMag(const Optional<double> &brightestMag)
    {
        m_BrightestStarMag = brightestMag;
    }

    void setConcentrationDeg(const QString &concentrationDeg)
    {
        m_ConcentrationDeg = concentrationDeg;
    }

private:
    Optional<double> m_BrightestStarMag;
    QString m_ConcentrationDeg;
};


class DsGalacticNebulaObsTarget : public DsObsTarget
{
public:
    DsGalacticNebulaObsTarget(const int id, const QString &name);

    DsGalacticNebulaObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                              const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                              const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                              const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                              const double surfBrightness, const QString &nebulaType, double posAngle);

    QString nebulaType() const
    {
        return m_NebulaType;
    }

    Optional<double> positionAngle() const
    {
        return m_PositionAngle;
    }

    void setNebulaType(const QString &nebulaType)
    {
        m_NebulaType = nebulaType;
    }

    void setPositionAngle(const Optional<double> &posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    QString m_NebulaType;
    Optional<double> m_PositionAngle;
};


class DsGalaxyObsTarget : public DsObsTarget
{
public:
    DsGalaxyObsTarget(const int id, const QString &name);

    DsGalaxyObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                      const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                      const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                      const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                      const double surfBrightness, const QString &hubbleType, const double posAngle);

    QString hubbleType() const
    {
        return m_HubbleType;
    }

    Optional<double> positionAngle() const
    {
        return m_PositionAngle;
    }

    void setHubbleType(const QString &hubbleType)
    {
        m_HubbleType = hubbleType;
    }

    void setPositionAngle(const Optional<double> &posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    QString m_HubbleType;
    Optional<double> m_PositionAngle;

};


class DsOpenClusterObsTarget : public DsObsTarget
{
public:
    DsOpenClusterObsTarget(const int id, const QString &name);

    DsOpenClusterObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                           const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                           const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                           const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                           const double surfBrightness, const int starCount, const double brightestStarMag,
                           const QString &classification);

    Optional<int> starCount() const
    {
        return m_StarCount;
    }

    Optional<double> brightestStarMagnitude() const
    {
        return m_BrightestStarMag;
    }

    QString classification() const
    {
        return m_Classification;
    }

    void setStarCount(const Optional<int> &starCount)
    {
        m_StarCount = starCount;
    }

    void setBrightestStarMagnitude(const Optional<double> &brightestStarMag)
    {
        m_BrightestStarMag = brightestStarMag;
    }

    void setClassification(const QString &classification)
    {
        m_Classification = classification;
    }

private:
    Optional<int> m_StarCount;
    Optional<double> m_BrightestStarMag;
    QString m_Classification;
};


class DsPlanetaryNebulaObsTarget : public DsObsTarget
{
public:
    DsPlanetaryNebulaObsTarget(const int id, const QString &name);

    DsPlanetaryNebulaObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                               const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                               const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                               const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                               const double surfBrightness, const double centralStarMag);

    Optional<double> centralStarMagnitude() const
    {
        return m_CentralStarMag;
    }

    void setCentralStarMagnitude(const Optional<double> &centralStarMag)
    {
        m_CentralStarMag = centralStarMag;
    }

private:
    Optional<double> m_CentralStarMag;
};


class DsQuasarObsTarget : public DsObsTarget
{
public:
    DsQuasarObsTarget(const int id, const QString &name);

    DsQuasarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                      const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                      const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                      const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                      const double surfBrightness);

};

class DsStarCloudObsTarget : public DsObsTarget
{
public:
    DsStarCloudObsTarget(const int id, const QString &name);

    DsStarCloudObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                         const int discovererObserverId, const dms &ra, const dms &dec, const REF_FRAME_ORIGIN origin,
                         const REF_FRAME_EQUINOX equinox, const QString &constellation, const QString &notes,
                         const dms &smallDiameter, const dms &largeDiameter, const double visMagnitude,
                         const double surfBrightness, const double posAngle);

    Optional<double> positionAngle() const
    {
        return m_PositionAngle;
    }

    void setPositionAngle(const Optional<double> &posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    Optional<double> m_PositionAngle;
};

}

#endif // DSOBSTARGETS_H
