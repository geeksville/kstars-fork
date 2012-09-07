/***************************************************************************
                          ksutils.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Mon Jan  7 10:48:09 EST 2002
    copyright            : (C) 2002 by Mark Hollomon
    email                : mhh@mindspring.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ksutils.h"
#include "Options.h"

#include <KStandardDirs>
#include <KUrl>

#include <QFile>

bool KSUtils::openDataFile( QFile &file, const QString &s ) {
    QString FileName = KStandardDirs::locate( "appdata", s );
    if ( !FileName.isNull() ) {
        file.setFileName( FileName );
        return file.open( QIODevice::ReadOnly );
    }
    return false;
}
