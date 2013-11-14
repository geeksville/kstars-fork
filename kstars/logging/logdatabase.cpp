/***************************************************************************
                    logdatabase.cpp - K Desktop Planetarium
                             -------------------
    begin                : Fri Oct 11 2013
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


#include "logdatabase.h"

#include "QSqlQuery"
#include "QSqlError"
#include "QFile"
#include "QStringList"

#include "KDebug"
#include "KLocale"
#include "KStandardDirs"

LogDatabase::LogDatabase()
{
    m_LogDatabase = QSqlDatabase::addDatabase("QSQLITE", "LogDatabase");
    QString dbFileName = KStandardDirs::locateLocal("appdata", "LogDatabase.sqlite");
    QFile dbFile(dbFileName);
    bool firstRun = false;
    if(!dbFile.exists()) {
        kWarning()<< i18n("Log Database does not exist! New User DB will be created.");
        firstRun = true;
    }

    m_LogDatabase.setDatabaseName(dbFileName);
    if(!m_LogDatabase.open()) {
        kWarning() << i18n("Error encountered while opening Log Database.");
    }

    if(firstRun) {
        // Create database structure
        createTables();
    }
}

LogDatabase::~LogDatabase()
{
    m_LogDatabase.close();
}

bool LogDatabase::createTables()
{
    QStringList queries;

    queries.append("create table observers("
                   "id int primary key not null,"
                   "name text not null,"
                   "surname text not null,"
                   "fstOffset real);");

    queries.append("create table contacts("
                   "id int primary key not null," // redundant?
                   "observerId int not null,"
                   "contact text not null);");

    queries.append("create table observerAccounts("
                   "id int primary key not null," // redundant?
                   "observerId int not null,"
                   "account text not null,"
                   "serviceName text not null);");

    queries.append("create table sites("
                   "id int primary key not null,"
                   "name text not null,"
                   "longitude text not null," // TODO: angleType
                   "latitude text not null," // TODO: angleType
                   "elevation real,"
                   "timezone integer default 0,"
                   "code integer);");

    queries.append("create table sessions("
                   "id int primary key not null,"
                   "begin text not null,"
                   "end text not null,"
                   "siteId int not null,"
                   "weather text,"
                   "equipment text,"
                   "comments text,"
                   "language text);"); // is it really required?

    queries.append("create table sessionCoObservers("
                   "sessionId int not null,"
                   "observerId int not null);");

    queries.append("create table sessionImages("
                   "sessionId int not null,"
                   "imageUrl text not null);");

    queries.append("create table targets("
                   "id int primary key not null,"
                   "datasource text,"
                   "observer text,"
                   "name text not null,"
                   "position text,"
                   "constellation text,"
                   "notes text);");

    queries.append("create table targetAliases("
                   "targetId int not null,"
                   "alias text not null);");

    // star targets

    queries.append("create table optics("
                   "id int primary key not null,"
                   "opticObjectType int not null,"
                   "type text,"
                   "vendor text,"
                   "aperture real,"
                   "lightGrasp real,"
                   "orientationErect int,"
                   "orientationTruesided int,"
                   "scopeFocalLength real,"
                   "fixedMagMagnification real,"
                   "fixedMagTrueFieldValue real,"
                   "fixedMagTrueFieldUnit text);");

    queries.append("create table eyepieces("
                   "id int primary key not null,"
                   "model text not null,"
                   "vendor text,"
                   "focalLength real not null,"
                   "maxFocalLength real,"
                   "apparentFovValue real,"
                   "apparentFovUnit text);");

    queries.append("create table lenses("
                   "id int primary key not null,"
                   "model text not null,"
                   "vendor text,"
                   "factor real not null);");

    queries.append("create table filters("
                   "id int primary key not null,"
                   "model text not null,"
                   "vendor text,"
                   "type text not null,"
                   "color text,"
                   "wratten text,"
                   "schott text);");

    queries.append("create table imagers("
                   "id int primary key not null,"
                   "imagerObjectType int not null,"
                   "model text not null,"
                   "vendor text,"
                   "remarks text,"
                   "pixelsX integer,"
                   "pixelXSize real,"
                   "pixelsY integer,"
                   "pixelYSize real,"
                   "binning int);");

    queries.append("create table observations("
                   "id int primary key not null,"
                   "observer int not null,"
                   "site int,"
                   "session int,"
                   "target int not null,"
                   "begin text not null,"
                   "end text,"
                   "faintestStar real,"
                   "skyQualityValue int not null,"
                   "skyQualityUnit text not null,"
                   "seeing int,"
                   "scope int,"
                   "accessories text,"
                   "eyepiece int,"
                   "lens int,"
                   "filter int,"
                   "magnification real,"
                   "imager int);");

    queries.append("create table observationImages("
                   "observationId int not null,"
                   "imageUrl text not null);");

    queries.append("create table observationFindings("
                   "observationId int not null,"
                   "description text not null,"
                   "language text);");

    QSqlQuery query(m_LogDatabase);
    foreach(QString queryString, queries) {
        query.exec(queryString);
    }

    kWarning() << query.lastError().databaseText();
    kWarning() << query.lastError().driverText();

    return true;
}
