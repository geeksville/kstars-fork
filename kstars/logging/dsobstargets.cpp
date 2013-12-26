/***************************************************************************
                    dsobstargets.cpp - K Desktop Planetarium
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

#include "dsobstargets.h"

using namespace Logging;

DsObsTarget::DsObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                         const int discovererObserverId, const dms &ra, const dms &dec, const QString &constellation,
                         const QString &notes, const dms &smallDiameter, const dms &largeDiameter,
                         const double visMagnitude, const double surfBrightness) :
    ObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes),
    m_SmallDiameter(smallDiameter), m_LargeDiameter(largeDiameter), m_VisualMagnitude(visMagnitude),
    m_SurfaceBrightness(surfBrightness)
{ }



DsMultipleStarObsTarget::DsMultipleStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                                 const int discovererObserverId, const dms &ra, const dms &dec,
                                                 const QString &constellation, const QString &notes, const QList<int> &componentsTargetIds) :
    ObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes),
    m_ComponentsTargetIds(componentsTargetIds)
{ }



DsAsterismObsTarget::DsAsterismObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                         const int discovererObserverId, const dms &ra, const dms &dec,
                                         const QString &constellation, const QString &notes, const dms &smallDiameter,
                                         const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                         const double posAngle) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_PositionAngle(posAngle)
{ }



DsGalaxyClusterObsTarget::DsGalaxyClusterObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                                   const int discovererObserverId, const dms &ra, const dms &dec,
                                                   const QString &constellation, const QString &notes, const dms &smallDiameter,
                                                   const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                                   const double brightest10thMag) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_Brightest10thMag(brightest10thMag)
{ }



DsDarkNebulaeObsTarget::DsDarkNebulaeObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                               const int discovererObserverId, const dms &ra, const dms &dec,
                                               const QString &constellation, const QString &notes, const dms &smallDiameter,
                                               const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                               const double posAngle, const int opacity) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_PositionAngle(posAngle), m_Opacity(opacity)
{ }



DsDoubleStarObsTarget::DsDoubleStarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                             const int discovererObserverId, const dms &ra, const dms &dec,
                                             const QString &constellation, const QString &notes, const dms &smallDiameter,
                                             const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                             const dms &separation, const double posAngle, const double companionMag) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_Separation(separation), m_PositionAngle(posAngle),
    m_CompanionMagnitude(companionMag)
{ }



DsGalacticNebulaObsTarget::DsGalacticNebulaObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                                     const int discovererObserverId, const dms &ra, const dms &dec,
                                                     const QString &constellation, const QString &notes, const dms &smallDiameter,
                                                     const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                                     const QString &nebulaType, double posAngle) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_NebulaType(nebulaType), m_PositionAngle(posAngle)
{ }



DsGalaxyObsTarget::DsGalaxyObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                     const int discovererObserverId, const dms &ra, const dms &dec,
                                     const QString &constellation, const QString &notes, const dms &smallDiameter,
                                     const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                     const QString &hubbleType, const double posAngle) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_HubbleType(hubbleType), m_PositionAngle(posAngle)
{ }



DsOpenClusterObsTarget::DsOpenClusterObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                               const int discovererObserverId, const dms &ra, const dms &dec,
                                               const QString &constellation, const QString &notes, const dms &smallDiameter,
                                               const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                               const int starCount, const double brightestStarMag, const QString &classification) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_StarCount(starCount), m_BrightestStarMag(brightestStarMag),
    m_Classification(classification)
{ }



DsPlanetaryNebulaObsTarget::DsPlanetaryNebulaObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                     const int discovererObserverId, const dms &ra, const dms &dec,
                                     const QString &constellation, const QString &notes, const dms &smallDiameter,
                                     const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                     const double centralStarMag) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_CentralStarMag(centralStarMag)
{ }



DsQuasarObsTarget::DsQuasarObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                     const int discovererObserverId, const dms &ra, const dms &dec,
                                     const QString &constellation, const QString &notes, const dms &smallDiameter,
                                     const dms &largeDiameter, const double visMagnitude, const double surfBrightness) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness)
{ }



DsStarCloudObsTarget::DsStarCloudObsTarget(const int id, const QString &name, const QStringList &aliases, const QString &datasource,
                                           const int discovererObserverId, const dms &ra, const dms &dec,
                                           const QString &constellation, const QString &notes, const dms &smallDiameter,
                                           const dms &largeDiameter, const double visMagnitude, const double surfBrightness,
                                           const double posAngle) :
    DsObsTarget(id, name, aliases, datasource, discovererObserverId, ra, dec, constellation, notes, smallDiameter,
                largeDiameter, visMagnitude, surfBrightness), m_PositionAngle(posAngle)
{ }
