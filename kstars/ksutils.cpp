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

#include <klocalizedstring.h>
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

QString KSUtils::toDirectionString( dms angle ) {
    // TODO: Instead of doing it this way, it would be nicer to
    // compute the string to arbitrary precision. Although that will
    // not be easy to localize.  (Consider, for instance, Indian
    // languages that have special names for the intercardinal points)
    // -- asimha

    static const char *directions[] = {
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "N"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "NNE"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "NE"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "ENE"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "E"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "ESE"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "SE"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "SSE"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "S"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "SSW"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "SW"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "WSW"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "W"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "WNW"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "NW"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "NNW"),
        I18N_NOOP2( "Abbreviated cardinal / intercardinal etc. direction", "N"),
    };

    int index = (int)( (angle.Degrees() + 11.25) / 22.5); // A number between 0 and 16 (inclusive), 16 meaning the same thing as zero.

    return i18nc( "Abbreviated cardinal / intercardinal etc. direction", directions[ index ] );
}
