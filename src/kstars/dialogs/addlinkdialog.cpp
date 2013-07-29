/***************************************************************************
                          addlinkdialog.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Oct 21 2001
    copyright            : (C) 2001 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "addlinkdialog.h"

#include <kurl.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <ktoolinvocation.h>

#include "skyobjects/skyobject.h"

AddLinkDialogUI::AddLinkDialogUI( QWidget *parent ) : QFrame( parent ) {
    setupUi(this);
}

AddLinkDialog::AddLinkDialog( QWidget *parent, const QString &oname )
        : KDialog( parent ),  ObjectName( oname )
{
    ald = new AddLinkDialogUI(this);
    setMainWidget( ald );
    setCaption( i18n( "Add Custom URL to %1", oname ) );
    setButtons( KDialog::Ok|KDialog::Cancel );

    //connect signals to slots
    connect( ald->URLButton, SIGNAL( clicked() ), this, SLOT( checkURL() ) );
    connect( ald->TypeBox, SIGNAL( clicked( int ) ), this, SLOT( changeDefaultDescription( int ) ) );

    ald->ImageRadio->setChecked(true);
    ald->DescBox->setText( i18n( "Show image of " ) + ObjectName );
}

void AddLinkDialog::checkURL( void ) {
    KUrl _url ( url() );
    if ( _url.isValid() ) {   //Is the string a valid URL?
        KToolInvocation::invokeBrowser( _url.url() );   //If so, launch the browser to see if it's the correct document
    } else {   //If not, print a warning message box that offers to open the browser to a search engine.
        QString message = i18n( "The URL is not valid. Would you like to open a browser window\nto the Google search engine?" );
        QString caption = i18n( "Invalid URL" );
        if ( KMessageBox::warningYesNo( 0, message, caption, KGuiItem(i18n("Browse Google")), KGuiItem(i18n("Do Not Browse")) )==KMessageBox::Yes ) {
            KToolInvocation::invokeBrowser( "http://www.google.com" );
        }
    }
}

void AddLinkDialog::changeDefaultDescription( int id ) {
    //If the user hasn't changed the default desc text, but the link type (image/webpage)
    //has been toggled, update the default desc text
    if ( id==1 && desc().startsWith( i18n( "Show image of " ) ) ) {
        ald->DescBox->setText( i18n( "Show webpage about " ) + ObjectName );
    }

    if ( id==0 && desc().startsWith( i18n( "Show webpage about " ) ) ) {
        ald->DescBox->setText( i18n( "Show image of " ) + ObjectName );
    }
}

#include "addlinkdialog.moc"
