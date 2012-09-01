/***************************************************************************
                          kstarsactions.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Mon Feb 25 2002
    copyright            : (C) 2002 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//needed in slotRunScript() for chmod() syscall (remote script downloaded to temp file)

#ifdef _WIN32
#include <windows.h>
#undef interface
#endif
#include <sys/stat.h>

#include <QCheckBox>
#include <QDir>
#include <QTextStream>
#include <QDialog>

#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <ktoggleaction.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <ktip.h>
#include <kstandarddirs.h>
#include <kconfigdialog.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <kmenu.h>
#include <kstatusbar.h>
#include <kprocess.h>
#include <ktoolbar.h>
#include <kicon.h>
#include <knewstuff3/downloaddialog.h>

#include "options/opscatalog.h"
#include "options/opsguides.h"
#include "options/opssolarsystem.h"
#include "options/opssatellites.h"
#include "options/opssupernovae.h"
#include "options/opscolors.h"
#include "options/opsadvanced.h"

#include "Options.h"
#include "kstars.h"
#include "kstarsdata.h"
#include "kstarsdatetime.h"
#include "skymap.h"
#include "skyobjects/skyobject.h"
#include "skyobjects/ksplanetbase.h"
#include "simclock.h"
#include "dialogs/timedialog.h"
#include "dialogs/locationdialog.h"
#include "dialogs/finddialog.h"
#include "dialogs/focusdialog.h"
#include "dialogs/fovdialog.h"
#include "printing/printingwizard.h"
#include "kswizard.h"
#include "tools/astrocalc.h"
#include "tools/altvstime.h"
#include "tools/wutdialog.h"
#include "tools/skycalendar.h"
#include "tools/scriptbuilder.h"
#include "tools/planetviewer.h"
#include "tools/jmoontool.h"
#include "tools/flagmanager.h"
#include "oal/execute.h"
#include "projections/projector.h"

#include <config-kstars.h>

#ifdef HAVE_INDI_H
//#include "ui_devmanager.h"
//#include "indi/indimenu.h"
//#include "indi/indidriver.h"
//#include "indi/imagesequence.h"

#include "ekos/ekosmanager.h"
#include "indi/telescopewizardprocess.h"
#include "indi/opsindi.h"
#include "indi/drivermanager.h"
#include "indi/guimanager.h"

#endif

#include "skycomponents/customcatalogcomponent.h"
#include "skycomponents/skymapcomposite.h"
#include "skycomponents/solarsystemcomposite.h"
#include "skycomponents/cometscomponent.h"
#include "skycomponents/asteroidscomponent.h"
#include "skycomponents/supernovaecomponent.h"

#ifdef HAVE_CFITSIO_H
#include "fitsviewer/fitsviewer.h"
#ifdef HAVE_INDI_H
//#include "ekos/ekos.h"
#endif
#endif

#ifdef HAVE_XPLANET
#include "xplanet/opsxplanet.h"
#endif

// #include "libkdeedu/kdeeduui/kdeeduglossary.h"

//This file contains function definitions for Actions declared in kstars.h

/** ViewToolBar Action.  All of the viewToolBar buttons are connected to this slot. **/

void KStars::slotViewToolBar() {
    KToggleAction *a = (KToggleAction*)sender();
    KConfigDialog *kcd = KConfigDialog::exists( "settings" );

    if ( a == actionCollection()->action( "show_stars" ) ) {
        Options::setShowStars( a->isChecked() );
        if ( kcd ) {
            opcatalog->kcfg_ShowStars->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_deepsky" ) ) {
        Options::setShowDeepSky( a->isChecked() );
        if ( kcd ) {
            opcatalog->kcfg_ShowDeepSky->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_planets" ) ) {
        Options::setShowSolarSystem( a->isChecked() );
        if ( kcd ) {
            opsolsys->kcfg_ShowSolarSystem->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_clines" ) ) {
        Options::setShowCLines( a->isChecked() );
        if ( kcd ) {
            opguides->kcfg_ShowCLines->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_cnames" ) ) {
        Options::setShowCNames( a->isChecked() );
        if ( kcd ) {
            opguides->kcfg_ShowCNames->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_cbounds" ) ) {
        Options::setShowCBounds( a->isChecked() );
        if ( kcd ) {
            opguides->kcfg_ShowCBounds->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_mw" ) ) {
        Options::setShowMilkyWay( a->isChecked() );
        if ( kcd ) {
            opguides->kcfg_ShowMilkyWay->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_equatorial_grid" ) ) {
        Options::setShowEquatorialGrid( a->isChecked() );
        if ( kcd ) {
            opguides->kcfg_ShowEquatorialGrid->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_horizontal_grid" ) ) {
        Options::setShowHorizontalGrid( a->isChecked() );
        if ( kcd ) {
            opguides->kcfg_ShowHorizontalGrid->setChecked( a->isChecked() );
        }    
    } else if ( a == actionCollection()->action( "show_horizon" ) ) {
        Options::setShowGround( a->isChecked() );
        if( !a->isChecked() && Options::useRefraction() ) {
           QString caption = i18n( "Refraction effects disabled" );
           QString message = i18n( "When the horizon is switched off, refraction effects are temporarily disabled." );
    
           KMessageBox::information( this, message, caption, "dag_refract_hide_ground" );
        }
        if ( kcd ) {
            opguides->kcfg_ShowGround->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_flags" ) ) {
        Options::setShowFlags( a->isChecked() );
        if ( kcd ) {
            opguides->kcfg_ShowFlags->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_satellites" ) ) {
        Options::setShowSatellites( a->isChecked() );
        if ( kcd ) {
            opssatellites->kcfg_ShowSatellites->setChecked( a->isChecked() );
        }
    } else if ( a == actionCollection()->action( "show_supernovae" ) ) {
        Options::setShowSupernovae( a->isChecked() );
        if ( kcd ) {
            opssupernovae->kcfg_ShowSupernovae->setChecked ( a->isChecked() ) ;
        }
    }

    // update time for all objects because they might be not initialized
    // it's needed when using horizontal coordinates
    data()->setFullTimeUpdate();
    updateTime();

    map()->forceUpdate();
}

/** Major Dialog Window Actions **/

void KStars::slotCalculator() {
    if( ! astrocalc )
        astrocalc = new AstroCalc (this);
    astrocalc->show();
}

void KStars::slotWizard() {
    QPointer<KSWizard> wizard = new KSWizard(this);
    if ( wizard->exec() == QDialog::Accepted ) {
        Options::setRunStartupWizard( false );  //don't run on startup next time

        data()->setLocation( *(wizard->geo()) );

        // adjust local time to keep UT the same.
        // create new LT without DST offset
        KStarsDateTime ltime = data()->geo()->UTtoLT( data()->ut() );

        // reset timezonerule to compute next dst change
        data()->geo()->tzrule()->reset_with_ltime( ltime, data()->geo()->TZ0(), data()->isTimeRunningForward() );

        // reset next dst change time
        data()->setNextDSTChange( data()->geo()->tzrule()->nextDSTChange() );

        // reset local sideral time
        data()->syncLST();

        // Make sure Numbers, Moon, planets, and sky objects are updated immediately
        data()->setFullTimeUpdate();

        // If the sky is in Horizontal mode and not tracking, reset focus such that
        // Alt/Az remain constant.
        if ( ! Options::isTracking() && Options::useAltAz() ) {
            map()->focus()->HorizontalToEquatorial( data()->lst(), data()->geo()->lat() );
        }

        // recalculate new times and objects
        data()->setSnapNextFocus();
        updateTime();
    }
    delete wizard;
}

void KStars::slotDownload() {
    KNS3::DownloadDialog dlg(this);
    dlg.exec();

    // Get the list of all the installed entries.
    KNS3::Entry::List entries = dlg.installedEntries();

    foreach (const KNS3::Entry &entry, entries) {
        foreach (const QString &name, entry.installedFiles()) {
            if ( name.endsWith( QLatin1String( ".cat" ) ) ) {
                // To start displaying the custom catalog, add it to SkyMapComposite
                Options::setCatalogFile(Options::catalogFile() << name);
                Options::setShowCatalog(Options::showCatalog() << 1);
                data()->skyComposite()->addCustomCatalog(name, Options::catalogFile().size() - 1);
            }
        }
    }
}

void KStars::slotAVT() {
    if ( ! avt ) avt = new AltVsTime(this);
    avt->show();
}

void KStars::slotWUT() {
    if ( ! wut ) wut = new WUTDialog(this);
    wut->show();
}

void KStars::slotCalendar() {
    if ( ! skycal ) skycal = new SkyCalendar(this);
    skycal->show();
}

void KStars::slotGlossary(){
    // 	GlossaryDialog *dlg = new GlossaryDialog( this, true );
    // 	QString glossaryfile =data()->stdDirs->findResource( "data", "kstars/glossary.xml" );
    // 	KUrl u = glossaryfile;
    // 	Glossary *g = new Glossary( u );
    // 	g->setName( i18n( "Knowledge" ) );
    // 	dlg->addGlossary( g );
    // 	dlg->show();
}

void KStars::slotScriptBuilder() {
    if ( ! sb ) sb = new ScriptBuilder(this);
    sb->show();
}

void KStars::slotSolarSystem() {
    if ( ! pv ) pv = new PlanetViewer(this);
    pv->show();
}

void KStars::slotJMoonTool() {
    if ( ! jmt ) jmt = new JMoonTool(this);
    jmt->show();
}

void KStars::slotFlagManager() {
    if ( ! fm ) fm = new FlagManager(this);
    fm->show();
}

void KStars::slotTelescopeWizard()
{
#ifdef HAVE_INDI_H
    if (KStandardDirs::findExe("indiserver").isEmpty())
    {
        KMessageBox::error(NULL, i18n("Unable to find INDI server. Please make sure the package that provides the 'indiserver' binary is installed."));
        return;
    }

    QPointer<telescopeWizardProcess> twiz = new telescopeWizardProcess(this);
    twiz->exec();
    delete twiz;
#endif
}

void KStars::slotINDIPanel() 
{
#ifdef HAVE_INDI_H
    if (KStandardDirs::findExe("indiserver").isEmpty())
    {
        KMessageBox::error(NULL, i18n("Unable to find INDI server. Please make sure the package that provides the 'indiserver' binary is installed."));
        return;
    }
    GUIManager::Instance()->updateStatus();
#endif
}

void KStars::slotINDIDriver() 
{
#ifdef HAVE_INDI_H
    if (KStandardDirs::findExe("indiserver").isEmpty())
    {
        KMessageBox::error(NULL, i18n("Unable to find INDI server. Please make sure the package that provides the 'indiserver' binary is installed."));
        return;
    }

    DriverManager::Instance()->show();
#endif
}

void KStars::slotEkos()
{
#ifdef HAVE_CFITSIO_H
#ifdef HAVE_INDI_H

    if (KStandardDirs::findExe("indiserver").isEmpty())
    {
        KMessageBox::error(NULL, i18n("Unable to find INDI server. Please make sure the package that provides the 'indiserver' binary is installed."));
        return;
    }

    if (ekosmenu == NULL)
        ekosmenu = new EkosManager();

    ekosmenu->show();
#endif
#endif
}

void KStars::slotGeoLocator() {
    QPointer<LocationDialog> locationdialog = new LocationDialog(this);
    if ( locationdialog->exec() == QDialog::Accepted ) {
        GeoLocation *newLocation = locationdialog->selectedCity();
        if ( newLocation ) {
            // set new location in options
            data()->setLocation( *newLocation );

            // adjust local time to keep UT the same.
            // create new LT without DST offset
            KStarsDateTime ltime = newLocation->UTtoLT( data()->ut() );

            // reset timezonerule to compute next dst change
            newLocation->tzrule()->reset_with_ltime( ltime, newLocation->TZ0(), data()->isTimeRunningForward() );

            // reset next dst change time
            data()->setNextDSTChange( newLocation->tzrule()->nextDSTChange() );

            // reset local sideral time
            data()->syncLST();

            // Make sure Numbers, Moon, planets, and sky objects are updated immediately
            data()->setFullTimeUpdate();

            // If the sky is in Horizontal mode and not tracking, reset focus such that
            // Alt/Az remain constant.
            if ( ! Options::isTracking() && Options::useAltAz() ) {
                map()->focus()->HorizontalToEquatorial( data()->lst(), data()->geo()->lat() );
            }

            // recalculate new times and objects
            data()->setSnapNextFocus();
            updateTime();
        }
    }
    delete locationdialog;
}

void KStars::slotViewOps() {
    //An instance of your dialog could be already created and could be cached,
    //in which case you want to display the cached dialog instead of creating
    //another one
    if ( KConfigDialog::showDialog( "settings" ) ) return;

    //KConfigDialog didn't find an instance of this dialog, so lets create it :
    KConfigDialog* dialog = new KConfigDialog( this, "settings",
                            Options::self() );

    connect( dialog, SIGNAL( settingsChanged( const QString &) ), this, SLOT( slotApplyConfigChanges() ) );

    opcatalog    = new OpsCatalog( this );
    opguides     = new OpsGuides( this );
    opsolsys     = new OpsSolarSystem( this );
    opssatellites= new OpsSatellites( this );
    opssupernovae= new OpsSupernovae( this );
    opcolors     = new OpsColors( this );
    opadvanced   = new OpsAdvanced( this );

    dialog->addPage(opcatalog, i18n("Catalogs"), "kstars_catalog");
    dialog->addPage(opsolsys, i18n("Solar System"), "kstars_solarsystem");
    dialog->addPage(opssatellites, i18n("Satellites"), "kstars_satellites");
    dialog->addPage(opssupernovae, i18n("Supernovae"), "kstars_supernovae");
    dialog->addPage(opguides, i18n("Guides"), "kstars_guides");
    dialog->addPage(opcolors, i18n("Colors"), "kstars_colors");

    #ifdef HAVE_INDI_H
    opsindi = new OpsINDI (this);
    dialog->addPage(opsindi, i18n("INDI"), "kstars");
    #endif

#ifdef HAVE_XPLANET
    opsxplanet = new OpsXplanet( this );
    dialog->addPage(opsxplanet, i18n("Xplanet"), "kstars_xplanet");
#endif

    dialog->addPage(opadvanced, i18n("Advanced"), "kstars_advanced");

    dialog->setHelp(QString(), "kstars");
    dialog->show();
}

void KStars::slotApplyConfigChanges() {
    Options::self()->writeConfig();

    // If the focus object was a constellation and the sky culture has changed, remove the focus object
    if( map()->focusObject() && map()->focusObject()->type() == SkyObject::CONSTELLATION ) {
        if( kstarsData->skyComposite()->currentCulture() != kstarsData->skyComposite()->getCultureName( (int)Options::skyCulture() ) || kstarsData->skyComposite()->isLocalCNames() != Options::useLocalConstellNames() ) {
            map()->setClickedObject( NULL );
            map()->setFocusObject( NULL );
        }
    }

    applyConfig();
    data()->setFullTimeUpdate();
    map()->forceUpdate();

    kstarsData->skyComposite()->setCurrentCulture(  kstarsData->skyComposite()->getCultureName( (int)Options::skyCulture() ) );
    kstarsData->skyComposite()->reloadCLines();
    kstarsData->skyComposite()->reloadCNames();
}

void KStars::slotSetTime() {
    QPointer<TimeDialog> timedialog = new TimeDialog( data()->lt(), data()->geo(), this );

    if ( timedialog->exec() == QDialog::Accepted ) {
        data()->changeDateTime( data()->geo()->LTtoUT( timedialog->selectedDateTime() ) );

        if ( Options::useAltAz() ) {
            if ( map()->focusObject() ) {
                map()->focusObject()->EquatorialToHorizontal( data()->lst(), data()->geo()->lat() );
                map()->setFocus( map()->focusObject() );
            } else
                map()->focus()->HorizontalToEquatorial( data()->lst(), data()->geo()->lat() );
        }

        map()->forceUpdateNow();

        //If focusObject has a Planet Trail, clear it and start anew.
        KSPlanetBase* planet = dynamic_cast<KSPlanetBase*>( map()->focusObject() );
        if( planet && planet->hasTrail() ) {
            planet->clearTrail();
            planet->addToTrail();
        }
    }
    delete timedialog;
}

//Set Time to CPU clock
void KStars::slotSetTimeToNow() {
    data()->changeDateTime( KStarsDateTime::currentUtcDateTime() );

    if ( Options::useAltAz() ) {
        if ( map()->focusObject() ) {
            map()->focusObject()->EquatorialToHorizontal( data()->lst(), data()->geo()->lat() );
            map()->setFocus( map()->focusObject() );
        } else
            map()->focus()->HorizontalToEquatorial( data()->lst(), data()->geo()->lat() );
    }

    map()->forceUpdateNow();

    //If focusObject has a Planet Trail, clear it and start anew.
    KSPlanetBase* planet = dynamic_cast<KSPlanetBase*>( map()->focusObject() );
    if( planet && planet->hasTrail() ) {
        planet->clearTrail();
        planet->addToTrail();
    }
}

void KStars::slotFind() {
    clearCachedFindDialog();
    if ( !findDialog ) {	  // create new dialog if no dialog is existing
        findDialog = new FindDialog( this );
    }

    if ( !findDialog ) kWarning() << i18n( "KStars::slotFind() - Not enough memory for dialog" ) ;
    SkyObject *targetObject;
    if ( findDialog->exec() == QDialog::Accepted && ( targetObject = findDialog->selectedObject() ) ) {
        map()->setClickedObject( targetObject );
        map()->setClickedPoint( map()->clickedObject() );
        map()->slotCenter();
    }

    // check if data has changed while dialog was open
    if ( DialogIsObsolete )
        clearCachedFindDialog();
}

void KStars::slotOpenFITS()
{
#ifdef HAVE_CFITSIO_H

    KUrl fileURL = KFileDialog::getOpenUrl( QDir::homePath(), "*.fits *.fit *.fts|Flexible Image Transport System" );

    if (fileURL.isEmpty())
        return;

    FITSViewer * fv = new FITSViewer(this);
    // Error opening file
    if (fv->addFITS(&fileURL) == -2)
        delete (fv);
    else
       fv->show();
#endif
}

void KStars::slotExportImage() {
    KUrl fileURL = KFileDialog::getSaveUrl( QDir::homePath(), "image/png image/jpeg image/gif image/x-portable-pixmap image/bmp image/svg+xml" );

    //User cancelled file selection dialog - abort image export
    if ( fileURL.isEmpty() ) {
        return;
    }

    //Warn user if file exists!
    if (QFile::exists(fileURL.path()))
    {
        int r=KMessageBox::warningContinueCancel(parentWidget(),
                i18n( "A file named \"%1\" already exists. Overwrite it?" , fileURL.fileName()),
                i18n( "Overwrite File?" ),
                KStandardGuiItem::overwrite() );
        if(r == KMessageBox::Cancel)
            return;
    }

    exportImage( fileURL.url(), map()->width(), map()->height() );
}

void KStars::slotRunScript() {
    KUrl fileURL = KFileDialog::getOpenUrl( QDir::homePath(), "*.kstars|KStars Scripts (*.kstars)" );
    QFile f;
    QString fname;

    if ( fileURL.isValid() ) {
        if ( ! fileURL.isLocalFile() ) {
            //Warn the user about executing remote code.
            QString message = i18n( "Warning:  You are about to execute a remote shell script on your machine. " );
            message += i18n( "If you absolutely trust the source of this script, press Continue to execute the script; " );
            message += i18n( "to save the file without executing it, press Save; " );
            message += i18n( "to cancel the download, press Cancel. " );

            int result = KMessageBox::warningYesNoCancel( 0, message, i18n( "Really Execute Remote Script?" ),
                         KStandardGuiItem::cont(), KStandardGuiItem::save() );

            if ( result == KMessageBox::Cancel ) return;
            if ( result == KMessageBox::No ) { //save file
                KUrl saveURL = KFileDialog::getSaveUrl( QDir::homePath(), "*.kstars|KStars Scripts (*.kstars)" );
                KTemporaryFile tmpfile;
                tmpfile.open();

                while ( ! saveURL.isValid() ) {
                    message = i18n( "Save location is invalid. Try another location?" );
                    if ( KMessageBox::warningYesNo( 0, message, i18n( "Invalid Save Location" ), KGuiItem(i18n("Try Another")), KGuiItem(i18n("Do Not Try")) ) == KMessageBox::No ) return;
                    saveURL = KFileDialog::getSaveUrl( QDir::homePath(), "*.kstars|KStars Scripts (*.kstars)" );
                }

                if ( saveURL.isLocalFile() ) {
                    fname = saveURL.toLocalFile();
                } else {
                    fname = tmpfile.fileName();
                }

                if( KIO::NetAccess::download( fileURL, fname, this ) ) {
                    chmod( fname.toAscii(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH ); //make it executable

                    if ( tmpfile.fileName() == fname ) { //upload to remote location
                        if ( ! KIO::NetAccess::upload( tmpfile.fileName(), fileURL, this ) ) {
                            QString message = i18n( "Could not upload image to remote location: %1", fileURL.prettyUrl() );
                            KMessageBox::sorry( 0, message, i18n( "Could not upload file" ) );
                        }
                    }
                } else {
                    KMessageBox::sorry( 0, i18n( "Could not download the file." ), i18n( "Download Error" ) );
                }

                return;
            }
        }

        //Damn the torpedos and full speed ahead, we're executing the script!
        KTemporaryFile tmpfile;
        tmpfile.open();

        if ( ! fileURL.isLocalFile() ) {
            fname = tmpfile.fileName();
            if( KIO::NetAccess::download( fileURL, fname, this ) ) {
                chmod( fname.toAscii(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH );
                f.setFileName( fname );
            }
        } else {
            f.setFileName( fileURL.toLocalFile() );
        }

        if ( !f.open( QIODevice::ReadOnly) ) {
            QString message = i18n( "Could not open file %1", f.fileName() );
            KMessageBox::sorry( 0, message, i18n( "Could Not Open File" ) );
            return;
        }

        // Before we run the script, make sure that it's safe.  Each line must either begin with "#"
        // or begin with "dbus-send". INDI scripts are much more complicated, so this simple test is not
        // suitable. INDI Scripting will return in KDE 4.1

        QTextStream istream(&f);
        QString line;
        bool fileOK( true );

        while (  ! istream.atEnd() ) {
            line = istream.readLine();
            if ( line.left(1) != "#" && line.left(9) != "dbus-send")
            {
                fileOK = false;
                break;
            }
        }

        if ( ! fileOK )
        {
            int answer;
            answer = KMessageBox::warningContinueCancel( 0, i18n( "The selected script contains unrecognized elements, "
                     "indicating that it was not created using the KStars script builder. "
                     "This script may not function properly, and it may even contain malicious code. "
                     "Would you like to execute it anyway?" ),
                     i18n( "Script Validation Failed" ), KGuiItem( i18n( "Run Nevertheless" ) ), KStandardGuiItem::cancel(), "daExecuteScript" );
            if ( answer == KMessageBox::Cancel ) return;
        }

        //Add statusbar message that script is running
        statusBar()->changeItem( i18n( "Running script: %1", fileURL.fileName() ), 0 );

        KProcess p;
        p << f.fileName();
        p.start();
        if( !p.waitForStarted() )
            return;

        while ( !p.waitForFinished( 10 ) )
        {
            qApp->processEvents(); //otherwise tempfile may get deleted before script completes.
            if( p.state() != QProcess::Running )
                break;
        }

        statusBar()->changeItem( i18n( "Script finished."), 0 );
    }
}

void KStars::slotPrint() {
    bool switchColors(false);

    //Suggest Chart color scheme
    if ( data()->colorScheme()->colorNamed( "SkyColor" ) != "#FFFFFF" ) {
        QString message = i18n( "You can save printer ink by using the \"Star Chart\" "
                                "color scheme, which uses a white background. Would you like to "
                                "temporarily switch to the Star Chart color scheme for printing?" );

        int answer;
        answer = KMessageBox::questionYesNoCancel( 0, message, i18n( "Switch to Star Chart Colors?" ),
                 KGuiItem(i18n("Switch Color Scheme")), KGuiItem(i18n("Do Not Switch")), KStandardGuiItem::cancel(), "askAgainPrintColors" );

        if ( answer == KMessageBox::Cancel )
            return;
        if ( answer == KMessageBox::Yes )
            switchColors = true;
    }

    printImage( true, switchColors );
}

void KStars::slotPrintingWizard() {
    if(printingWizard) {
        delete printingWizard;
    }

    printingWizard = new PrintingWizard(this);
    printingWizard->show();
}

void KStars::slotToggleTimer() {
    if ( data()->clock()->isActive() ) {
        data()->clock()->stop();
        updateTime();
    } else {
        if ( fabs( data()->clock()->scale() ) > Options::slewTimeScale() )
            data()->clock()->setManualMode( true );
        data()->clock()->start();
        if ( data()->clock()->isManualMode() )
            map()->forceUpdate();
    }
    
    // Update clock state in options
    Options::setRunClock( data()->clock()->isActive() );
}

void KStars::slotStepForward() {
    if ( data()->clock()->isActive() )
        data()->clock()->stop();
    data()->clock()->manualTick( true );
    map()->forceUpdate();
}

void KStars::slotStepBackward() {
    if ( data()->clock()->isActive() )
        data()->clock()->stop();
    data()->clock()->setClockScale( -1.0 * data()->clock()->scale() ); //temporarily need negative time step
    data()->clock()->manualTick( true );
    data()->clock()->setClockScale( -1.0 * data()->clock()->scale() ); //reset original sign of time step
    map()->forceUpdate();
}

//Pointing
void KStars::slotPointFocus() {
    // In the following cases, we set slewing=true in order to disengage tracking
    map()->stopTracking();

    if ( sender() == actionCollection()->action("zenith") ) 
        map()->setDestinationAltAz( dms(90.0), map()->focus()->az() );
    else if ( sender() == actionCollection()->action("north") )
        map()->setDestinationAltAz( dms(15.0), dms(0.0001) );
    else if ( sender() == actionCollection()->action("east") )
        map()->setDestinationAltAz( dms(15.0), dms(90.0) );
    else if ( sender() == actionCollection()->action("south") )
        map()->setDestinationAltAz( dms(15.0), dms(180.0) );
    else if ( sender() == actionCollection()->action("west") )
        map()->setDestinationAltAz( dms(15.0), dms(270.0) );
}

void KStars::slotTrack() {
    if ( Options::isTracking() ) {
        Options::setIsTracking( false );
        actionCollection()->action("track_object")->setText( i18n( "Engage &Tracking" ) );
        actionCollection()->action("track_object")->setIcon( KIcon("document-decrypt") );

        KSPlanetBase* planet = dynamic_cast<KSPlanetBase*>( map()->focusObject() );
        if( planet && data()->temporaryTrail ) {
            planet->clearTrail();
            data()->temporaryTrail = false;
        }

        map()->setClickedObject( NULL );
        map()->setFocusObject( NULL );//no longer tracking focusObject
        map()->setFocusPoint( NULL );
    } else {
        map()->setClickedPoint( map()->focus() );
        map()->setClickedObject( NULL );
        map()->setFocusObject( NULL );//no longer tracking focusObject
        map()->setFocusPoint( map()->clickedPoint() );
        Options::setIsTracking( true );
        actionCollection()->action("track_object")->setText( i18n( "Stop &Tracking" ) );
        actionCollection()->action("track_object")->setIcon( KIcon("document-encrypt") );
    }

    map()->forceUpdate();
}

void KStars::slotManualFocus() {
    QPointer<FocusDialog> focusDialog = new FocusDialog( this ); // = new FocusDialog( this );
    if ( Options::useAltAz() ) focusDialog->activateAzAltPage();

    if ( focusDialog->exec() == QDialog::Accepted ) {
        //DEBUG
        kDebug() << "focusDialog point: " << &focusDialog;

        //If the requested position is very near the pole, we need to point first
        //to an intermediate location just below the pole in order to get the longitudinal
        //position (RA/Az) right.
        double realAlt( focusDialog->point().alt().Degrees() );
        double realDec( focusDialog->point().dec().Degrees() );
        if ( Options::useAltAz() && realAlt > 89.0 ) {
            focusDialog->point().setAlt( 89.0 );
            focusDialog->point().HorizontalToEquatorial( data()->lst(), data()->geo()->lat() );
        }
        if ( ! Options::useAltAz() && realDec > 89.0 ) {
            focusDialog->point().setDec( 89.0 );
            focusDialog->point().EquatorialToHorizontal( data()->lst(), data()->geo()->lat() );
        }

        map()->setClickedPoint( & focusDialog->point() );
        if ( Options::isTracking() ) slotTrack();

        map()->slotCenter();

        //The slew takes some time to complete, and this often causes the final focus point to be slightly
        //offset from the user's requested coordinates (because EquatorialToHorizontal() is called
        //throughout the process, which depends on the sidereal time).  So we now "polish" the final
        //position by resetting the final focus to the focusDialog point.
        //
        //Also, if the requested position was within 1 degree of the coordinate pole, this will
        //automatically correct the final pointing from the intermediate offset position to the final position
        data()->setSnapNextFocus();
        if ( Options::useAltAz() ) {
            map()->setDestinationAltAz( focusDialog->point().alt(), focusDialog->point().az() );
        } else {
            map()->setDestination( focusDialog->point().ra(), focusDialog->point().dec() );
        }

        //Now, if the requested point was near a pole, we need to reset the Alt/Dec of the focus.
        if ( Options::useAltAz() && realAlt > 89.0 ) map()->focus()->setAlt( realAlt );
        if ( ! Options::useAltAz() && realDec > 89.0 ) map()->focus()->setDec( realAlt );

        //Don't track if we set Alt/Az coordinates.  This way, Alt/Az remain constant.
        if ( focusDialog->usedAltAz() )
            map()->stopTracking();
    }
    delete focusDialog;
}

void KStars::slotZoomChanged() {
    // Enable/disable actions
    actionCollection()->action("zoom_out")->setEnabled( Options::zoomFactor() > MINZOOM );
    actionCollection()->action("zoom_in" )->setEnabled( Options::zoomFactor() < MAXZOOM );
    // Update status bar
    float fov = map()->projector()->fov();
    QString fovunits = i18n( "degrees" );
    if ( fov < 1.0 ) {
        fov = fov * 60.0;
        fovunits = i18n( "arcminutes" );
    } 
    if ( fov < 1.0 ) {
        fov = fov * 60.0;
        fovunits = i18n( "arcseconds" );
    }
    QString fovstring = i18nc("field of view", "FOV") + ": " + QString::number( fov, 'f', 1 ) + ' ' + fovunits;
    statusBar()->changeItem( fovstring, 0 );
}

void KStars::slotSetZoom() {
    bool ok;
    double currentAngle = map()->width() / ( Options::zoomFactor() * dms::DegToRad );
    double minAngle = map()->width() / ( MAXZOOM * dms::DegToRad );
    double maxAngle = map()->width() / ( MINZOOM * dms::DegToRad );

    double angSize = KInputDialog::getDouble( i18nc( "The user should enter an angle for the field-of-view of the display",
                                                     "Enter Desired Field-of-View Angle" ),
                                              i18n( "Enter a field-of-view angle in degrees: " ),
                                              currentAngle, minAngle, maxAngle, 0.1, 1, &ok );

    if( ok ) {
        map()->setZoomFactor( map()->width() / ( angSize * dms::DegToRad ) );
    }
}

void KStars::slotCoordSys() {
    if ( Options::useAltAz() ) {
        Options::setUseAltAz( false );
        if ( Options::useRefraction() ) {
            if ( map()->focusObject() ) //simply update focus to focusObject's position
                map()->setFocus( map()->focusObject() );
            else { //need to recompute focus for unrefracted position
                map()->setFocusAltAz( SkyPoint::unrefract( map()->focus()->alt() ),
                                      map()->focus()->az() );
                map()->focus()->HorizontalToEquatorial( data()->lst(), data()->geo()->lat() );
            }
        }
        actionCollection()->action("coordsys")->setText( i18n("Switch to horizonal view (Horizontal &Coordinates)") );
    } else {
        Options::setUseAltAz( true );
        if ( Options::useRefraction() ) {
            map()->setFocusAltAz( map()->focus()->altRefracted(), map()->focus()->az() );
        }
        actionCollection()->action("coordsys")->setText( i18n("Switch to star globe view (Equatorial &Coordinates)") );
    }
    map()->forceUpdate();
}

void KStars::slotMapProjection() {
    if ( sender() == actionCollection()->action("project_lambert") )
        Options::setProjection( SkyMap::Lambert );
    if ( sender() == actionCollection()->action("project_azequidistant") )
        Options::setProjection( SkyMap::AzimuthalEquidistant );
    if ( sender() == actionCollection()->action("project_orthographic") )
        Options::setProjection( SkyMap::Orthographic );
    if ( sender() == actionCollection()->action("project_equirectangular") )
        Options::setProjection( SkyMap::Equirectangular );
    if ( sender() == actionCollection()->action("project_stereographic") )
        Options::setProjection( SkyMap::Stereographic );
    if ( sender() == actionCollection()->action("project_gnomonic") )
        Options::setProjection( SkyMap::Gnomonic );

    //DEBUG
    kDebug() << i18n( "Projection system: %1", Options::projection() );

    skymap->forceUpdate();
}

//Settings Menu:
void KStars::slotColorScheme() {
    //use mid(3) to exclude the leading "cs_" prefix from the action name
    QString filename = QString( sender()->objectName() ).mid(3) + ".colors";
    loadColorScheme( filename );
}

void KStars::slotTargetSymbol(bool flag) {
    kDebug() << QString("slotTargetSymbol: %1 %2").arg( sender()->objectName() ).arg( flag);
    
    QStringList names = Options::fOVNames();
    if( flag ) {
        // Add FOV to list
        names.append( sender()->objectName() );
    } else {
        // Remove FOV from list
        int ix = names.indexOf( sender()->objectName() );
        if( ix >= 0 ) 
            names.removeAt(ix);
    }
    Options::setFOVNames( names );
   
    // Sync visibleFOVs with fovNames
    data()->syncFOV();
    
    map()->forceUpdate();
}

void KStars::slotFOVEdit() {
    QPointer<FOVDialog> fovdlg = new FOVDialog( this );
    if ( fovdlg->exec() == QDialog::Accepted ) {
        fovdlg->writeFOVList();
        repopulateFOV();
    }
    delete fovdlg;
}

void KStars::slotObsList() {
    obsList->show();
}

void KStars::slotEquipmentWriter() {
    eWriter->show();
}

void KStars::slotObserverAdd() {
    oAdd->show();
}

void KStars::slotExecute() {
    getExecute()->init();
    getExecute()->show();
}

//Help Menu
void KStars::slotTipOfDay() {
    KTipDialog::showTip(this, "kstars/tips", true);
}

// Toggle to and from full screen mode
void KStars::slotFullScreen()
{
    if ( topLevelWidget()->isFullScreen() ) {
        topLevelWidget()->setWindowState( topLevelWidget()->windowState() & ~Qt::WindowFullScreen ); // reset
    } else {
        topLevelWidget()->setWindowState( topLevelWidget()->windowState() | Qt::WindowFullScreen ); // set
    }
}

void KStars::slotClearAllTrails() {
    //Exclude object with temporary trail
    SkyObject *exOb( NULL );
    if ( map()->focusObject() && map()->focusObject()->isSolarSystem() && data()->temporaryTrail ) {
        exOb = map()->focusObject();
    }

    TrailObject::clearTrailsExcept( exOb );

    map()->forceUpdate();
}

//toggle display of GUI Items on/off
void KStars::slotShowGUIItem( bool show ) {
    //Toolbars
    if ( sender() == actionCollection()->action( "show_statusBar" ) ) {
        Options::setShowStatusBar( show );
        statusBar()->setVisible(show);
    }

    if ( sender() == actionCollection()->action( "show_sbAzAlt" ) ) {
        Options::setShowAltAzField( show );
        if( !show )
            statusBar()->changeItem( QString(), 1 );
    }

    if ( sender() == actionCollection()->action( "show_sbRADec" ) ) {
        Options::setShowRADecField( show );
        if( ! show )
            statusBar()->changeItem( QString(), 2 );
    }

}
void KStars::addColorMenuItem( const QString &name, const QString &actionName ) {
    KToggleAction *kta = actionCollection()->add<KToggleAction>( actionName );
    kta->setText( name );
    kta->setObjectName( actionName );
    kta->setActionGroup( cschemeGroup );
    connect( kta, SIGNAL( toggled( bool ) ), this, SLOT( slotColorScheme() ) );
    colorActionMenu->addAction( kta );

    KConfigGroup cg = KGlobal::config()->group( "Colors" );
    if ( actionName.mid( 3 ) == cg.readEntry( "ColorSchemeFile", "classic.colors" ).remove( ".colors" ) ) {
        kta->setChecked( true );
    }
}

void KStars::removeColorMenuItem( const QString &actionName ) {
    kDebug() << "removing " << actionName;
    colorActionMenu->removeAction( actionCollection()->action( actionName ) );
}

/*

void KStars::establishINDI()
{
#ifdef HAVE_INDI_H
    if (indimenu == NULL)
        indimenu = GUIManager::Instance();
    if (indidriver == NULL)
        indidriver = new DriverManager();
#endif
}
*/

void KStars::slotAboutToQuit()
{
    // Delete skymap. This required to run destructors and save
    // current state in the option.
    delete skymap;

    //Store Window geometry in Options object
    Options::setWindowWidth( width() );
    Options::setWindowHeight( height() );

    //explicitly save the colorscheme data to the config file
    data()->colorScheme()->saveToConfig();

    //synch the config file with the Config object
    writeConfig();

    if( !Options::obsListSaveImage() ) {
        foreach ( const QString& file, obsList->imageList() )
            QFile::remove( KStandardDirs::locateLocal( "appdata", file ) );
    }
}

void KStars::slotShowPositionBar(SkyPoint* p ) {
    if ( Options::showAltAzField() ) {
        dms a = p->alt();
        if ( Options::useAltAz() )
            a = p->altRefracted();
        QString s = QString("%1, %2").arg( p->az().toDMSString(true), //true: force +/- symbol
                                           a.toDMSString(true) );                 //true: force +/- symbol
        statusBar()->changeItem( s, 1 );
    }
    if ( Options::showRADecField() ) {
        QString s = QString("%1, %2").arg(p->ra().toHMSString(),
                                          p->dec().toDMSString(true) ); //true: force +/- symbol
        statusBar()->changeItem( s, 2 );
    }
}

void KStars::slotUpdateComets() {
    data()->skyComposite()->solarSystemComposite()->cometsComponent()->updateDataFile();
}

void KStars::slotUpdateAsteroids() {
    data()->skyComposite()->solarSystemComposite()->asteroidsComponent()->updateDataFile();
}

void KStars::slotUpdateSupernovae()
{
    data()->skyComposite()->supernovaeComponent()->updateDataFile();
}
