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

#include "obstarget.h"

namespace Logging
{

class DsObsTarget : public ObsTarget
{
public:
    DsObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                const int discovererObserverId, const dms &ra, const dms &dec,
                const QString &constellation, const QString &notes, const dms &smallDiameter,
                const dms &largeDiameter, const double visMagnitude, const double surfBrightness);

    dms smallDiameter() const
    {
        return m_SmallDiameter;
    }

    dms largeDiameter() const
    {
        return m_LargeDiameter;
    }

    double visualMagnitude() const
    {
        return m_VisualMagnitude;
    }

    double surfaceBrightness() const
    {
        return m_SurfaceBrightness;
    }

    void setSmallDiameter(const dms &smallDiameter)
    {
        m_SmallDiameter = smallDiameter;
    }

    void setLargeDiameter(const dms &largeDiameter)
    {
        m_LargeDiameter = largeDiameter;
    }

    void setVisualMagnitude(const double visualMagnitude)
    {
        m_VisualMagnitude = visualMagnitude;
    }

    void setSurfaceBrightness(const double surfaceBrightness)
    {
        m_SurfaceBrightness = surfaceBrightness;
    }

private:
    dms m_SmallDiameter;
    dms m_LargeDiameter;
    double m_VisualMagnitude;
    double m_SurfaceBrightness;
};


class DsMultipleStarObsTarget : public ObsTarget
{
public:
    DsMultipleStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                            const int discovererObserverId, const dms &ra, const dms &dec,
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
    DsAsterismObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                        const int discovererObserverId, const dms &ra, const dms &dec,
                        const QString &constellation, const QString &notes, const dms &smallDiameter,
                        const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                        const double posAngle);

    double positionAngle() const
    {
        return m_PositionAngle;
    }

    void setPositionAngle(const double posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    double m_PositionAngle;
};


class DsGalaxyClusterObsTarget : public DsObsTarget
{
public:
    DsGalaxyClusterObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                             const int discovererObserverId, const dms &ra, const dms &dec,
                             const QString &constellation, const QString &notes, const dms &smallDiameter,
                             const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                             const double brightest10thMag);

    double brightest10thMag() const
    {
        return m_Brightest10thMag;
    }

    void setBrightest10thMag(const double mag)
    {
        m_Brightest10thMag = mag;
    }

private:
    double m_Brightest10thMag;
};


class DsDarkNebulaeObsTarget : public DsObsTarget
{
public:
    DsDarkNebulaeObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                           const int discovererObserverId, const dms &ra, const dms &dec,
                           const QString &constellation, const QString &notes, const dms &smallDiameter,
                           const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                           const double posAngle, const int opacity);

    double positionAngle() const
    {
        return m_PositionAngle;
    }

    int opacity() const
    {
        return m_Opacity;
    }

    void setPositionAngle(const double posAngle)
    {
        m_PositionAngle = posAngle;
    }

    void setOpacity(const int opacity)
    {
        m_Opacity = opacity;
    }

private:
    double m_PositionAngle;
    int m_Opacity;
};


class DsDoubleStarObsTarget : public DsObsTarget
{
public:
    DsDoubleStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                          const int discovererObserverId, const dms &ra, const dms &dec,
                          const QString &constellation, const QString &notes, const dms &smallDiameter,
                          const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                          const dms &separation, const double posAngle, const double companionMag);

    dms separation() const
    {
        return m_Separation;
    }

    double positionAngle() const
    {
        return m_PositionAngle;
    }

    double companionMagnitude() const
    {
        return m_CompanionMagnitude;
    }

    void setSeparation(const dms &separation)
    {
        m_Separation = separation;
    }

    void setPositionAngle(const double posAngle)
    {
        m_PositionAngle = posAngle;
    }

    void setCompanionMagnitude(const double companionMag)
    {
        m_CompanionMagnitude = companionMag;
    }

private:
    dms m_Separation;
    double m_PositionAngle;
    double m_CompanionMagnitude;
};

class DsGalacticNebulaObsTarget : public DsObsTarget
{
public:
    DsGalacticNebulaObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                              const int discovererObserverId, const dms &ra, const dms &dec,
                              const QString &constellation, const QString &notes, const dms &smallDiameter,
                              const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                              const QString &nebulaType, double posAngle);

    QString nebulaType() const
    {
        return m_NebulaType;
    }

    double positionAngle() const
    {
        return m_PositionAngle;
    }

    void setNebulaType(const QString &nebulaType)
    {
        m_NebulaType = nebulaType;
    }

    void setPositionAngle(const double posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    QString m_NebulaType;
    double m_PositionAngle;
};

class DsGalaxyObsTarget : public DsObsTarget
{
public:
    DsGalaxyObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                      const int discovererObserverId, const dms &ra, const dms &dec,
                      const QString &constellation, const QString &notes, const dms &smallDiameter,
                      const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                      const QString &hubbleType, const double posAngle);

    QString hubbleType() const
    {
        return m_HubbleType;
    }

    double positionAngle() const
    {
        return m_PositionAngle;
    }

    void setHubbleType(const QString &hubbleType)
    {
        m_HubbleType = hubbleType;
    }

    void setPositionAngle(const double posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    QString m_HubbleType;
    double m_PositionAngle;

};

class DsOpenClusterObsTarget : public DsObsTarget
{
public:
    DsOpenClusterObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                           const int discovererObserverId, const dms &ra, const dms &dec,
                           const QString &constellation, const QString &notes, const dms &smallDiameter,
                           const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                           const int starCount, const double brightestStarMag, const QString &classification);

    int starCount() const
    {
        return m_StarCount;
    }

    double brightestStarMagnitude() const
    {
        return m_BrightestStarMag;
    }

    QString classification() const
    {
        return m_Classification;
    }

    void setStarCount(const int starCount)
    {
        m_StarCount = starCount;
    }

    void setBrightestStarMagnitude(const double brightestStarMag)
    {
        m_BrightestStarMag = brightestStarMag;
    }

    void setClassification(const QString &classification)
    {
        m_Classification = classification;
    }

private:
    int m_StarCount;
    double m_BrightestStarMag;
    QString m_Classification;
};

class DsPlanetaryNebulaObsTarget : public DsObsTarget
{
public:
    DsPlanetaryNebulaObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                      const int discovererObserverId, const dms &ra, const dms &dec,
                      const QString &constellation, const QString &notes, const dms &smallDiameter,
                      const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                      const double centralStarMag);

    double centralStarMagnitude() const
    {
        return m_CentralStarMag;
    }

    void setCentralStarMagnitude(const double centralStarMag)
    {
        m_CentralStarMag = centralStarMag;
    }

private:
    double m_CentralStarMag;
};

class DsQuasarObsTarget : public DsObsTarget
{
public:
    DsQuasarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                      const int discovererObserverId, const dms &ra, const dms &dec,
                      const QString &constellation, const QString &notes, const dms &smallDiameter,
                      const dms &largeDiameter, const double visMagnitude, const double surfBrightness);

};

class DsStarCloudObsTarget : public DsObsTarget
{
public:
    DsStarCloudObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                         const int discovererObserverId, const dms &ra, const dms &dec,
                         const QString &constellation, const QString &notes, const dms &smallDiameter,
                         const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                         const double posAngle);

    double positionAngle() const
    {
        return m_PositionAngle;
    }

    void setPositionAngle(const double posAngle)
    {
        m_PositionAngle = posAngle;
    }

private:
    double m_PositionAngle;
};

}

#endif // DSOBSTARGETS_H
