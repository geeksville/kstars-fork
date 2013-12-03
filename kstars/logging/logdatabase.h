/***************************************************************************
                    logdatabase.h - K Desktop Planetarium
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


#ifndef LOGDATABASE_H
#define LOGDATABASE_H

#include "QtSql/QSqlDatabase"

QT_BEGIN_NAMESPACE
class QSqlQuery;
QT_END_NAMESPACE

class LogDatabase
{
public:
    LogDatabase();
    ~LogDatabase();

    bool createTables();

private:
    QStringList baseTableCreationQueries();
    QStringList equipmentTableCreationQueries();
    QStringList varStarTableCreationQueries();
    QStringList dsoTableCreationQueries();
    QStringList solarSystemTableCreationQueries();
    QStringList imagerTableCreationQueries();

    bool executeQueries(const QStringList &queries, QSqlQuery &query);

    QSqlDatabase m_LogDatabase;
};

#endif // LOGDATABASE_H
