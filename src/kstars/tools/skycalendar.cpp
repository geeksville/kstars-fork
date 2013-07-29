/***************************************************************************
                          skycalendar.cpp  -  description
                             -------------------
    begin                : Wed Jul 16 2008
    copyright            : (C) 2008 by Jason Harris
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

#include "skycalendar.h"

#include <QDesktopWidget>
#include <QPixmap>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QFontInfo>
#include <kdeprintdialog.h>
#include <kdebug.h>
#include <KPlotObject>
#include <KPushButton>

#include "calendarwidget.h"
#include "geolocation.h"
#include "dialogs/locationdialog.h"
#include "kstarsdatetime.h"
#include "kstarsdata.h"
#include "skyobjects/ksplanet.h"
#include "skycomponents/skymapcomposite.h"

namespace {
    // convert time to decimal hours since midnight
    float timeToHours(QTime t) {
        float h = t.secsTo(QTime()) * -24.0 / 86400.0;
        if( h > 12.0 )
            h -= 24.0;
        return h;
    }

    // Check that axis has been crossed
    inline bool isAxisCrossed(const QVector<QPointF>& vec, int i) {
        return i > 0  &&  vec.at(i-1).x() * vec.at(i).x() <= 0;
    }
    // Check that we are at maximum
    inline bool isAtExtremum(const QVector<QPointF>& vec, int i) {
        return
            i > 0 && i < vec.size() - 1 &&
            (vec.at(i-1).x() - vec.at(i).x()) * (vec.at(i).x() - vec.at(i+1).x()) < 0;
    }
}

SkyCalendarUI::SkyCalendarUI( QWidget *parent )
    : QFrame( parent )
{
    setupUi( this );
}

SkyCalendar::SkyCalendar( QWidget *parent )
    : KDialog( parent )
{
    scUI = new SkyCalendarUI( this );
    setMainWidget( scUI );
    
    geo = KStarsData::Instance()->geo();

    setCaption( i18n( "Sky Calendar" ) );
    setButtons( KDialog::User1 | KDialog::Close );
    setModal( false );

    //Adjust minimum size for small screens:
    if ( QApplication::desktop()->availableGeometry().height() <= scUI->CalendarView->height() ) {
        scUI->CalendarView->setMinimumSize( 400, 600 );
    }
    
    scUI->CalendarView->setShowGrid( false ); 
    scUI->Year->setValue( KStarsData::Instance()->lt().date().year() );

    scUI->LocationButton->setText( geo->fullName() );
    setButtonGuiItem( KDialog::User1, KGuiItem( i18n("&Print..."), "document-print", i18n("Print the Sky Calendar") ) );
    
    scUI->CalendarView->setHorizon();

    connect( scUI->CreateButton, SIGNAL(clicked()), this, SLOT(slotFillCalendar()) );
    connect( scUI->LocationButton, SIGNAL(clicked()), this, SLOT(slotLocation()) );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotPrint() ) );
}

SkyCalendar::~SkyCalendar() {
}

int SkyCalendar::year()  { return scUI->Year->value(); }

void SkyCalendar::slotFillCalendar() {
    scUI->CalendarView->resetPlot();
    scUI->CalendarView->setHorizon();
    
    if ( scUI->checkBox_Mercury->isChecked() )
        addPlanetEvents( KSPlanetBase::MERCURY );
    if ( scUI->checkBox_Venus->isChecked() )
        addPlanetEvents( KSPlanetBase::VENUS );
    if ( scUI->checkBox_Mars->isChecked() )
        addPlanetEvents( KSPlanetBase::MARS );
    if ( scUI->checkBox_Jupiter->isChecked() )
        addPlanetEvents( KSPlanetBase::JUPITER );
    if ( scUI->checkBox_Saturn->isChecked() )
        addPlanetEvents( KSPlanetBase::SATURN );
    if ( scUI->checkBox_Uranus->isChecked() )
        addPlanetEvents( KSPlanetBase::URANUS );
    if ( scUI->checkBox_Neptune->isChecked() )
        addPlanetEvents( KSPlanetBase::NEPTUNE );
    if ( scUI->checkBox_Pluto->isChecked() )
        addPlanetEvents( KSPlanetBase::PLUTO );
    
    scUI->CalendarView->update();
}

// FIXME: For the time being, adjust with dirty, cluttering labels that don't align to the line
/*
void SkyCalendar::drawEventLabel( float x1, float y1, float x2, float y2, QString LabelText ) {
    QFont origFont = p->font();
    p->setFont( QFont( "Bitstream Vera", 10 ) );

    int textFlags = Qt::AlignCenter; // TODO: See if Qt::SingleLine flag works better
    QFontMetricsF fm( p->font(), p->device() );

    QRectF LabelRect = fm.boundingRect( QRectF(0,0,1,1), textFlags, LabelText );
    QPointF LabelPoint = scUI->CalendarView->mapToWidget( QPointF( x, y ) );
        
    float LabelAngle = atan2( y2 - y1, x2 - x1 )/dms::DegToRad;
        
    p->save();
    p->translate( LabelPoint );
    p->rotate( LabelAngle );
    p->drawText( LabelRect, textFlags, LabelText );
    p->restore();

    p->setFont( origFont );
}
*/

void SkyCalendar::addPlanetEvents( int nPlanet ) {
    KSPlanetBase *ksp = KStarsData::Instance()->skyComposite()->planet( nPlanet );
    QColor pColor = ksp->color();
    QVector<QPointF> vRise, vSet, vTransit;

    for( KStarsDateTime kdt( QDate( year(), 1, 1 ), QTime( 12, 0, 0 ) );
         kdt.date().year() == year();
         kdt = kdt.addDays( scUI->spinBox_Interval->value() ) )
    {
        float rTime, sTime, tTime;
        
        //Compute rise/set/transit times.  If they occur before noon, 
        //recompute for the following day
        QTime tmp_rTime = ksp->riseSetTime( kdt, geo, true, true );     //rise time, exact
        QTime tmp_sTime = ksp->riseSetTime( kdt, geo, false, true );    //set time, exact
        QTime tmp_tTime = ksp->transitTime( kdt, geo );
        QTime midday( 12, 0, 0 );
        
        // NOTE: riseSetTime should be fix now, this test is no longer necessary
        if ( tmp_rTime == tmp_sTime ) {
            tmp_rTime = QTime();
            tmp_sTime = QTime();
        }
        
        if ( tmp_rTime.isValid() && tmp_sTime.isValid() ) {
            rTime = tmp_rTime.secsTo( midday ) * 24.0 / 86400.0;
            sTime = tmp_sTime.secsTo( midday ) * 24.0 / 86400.0;
            
            if ( tmp_rTime <= midday )
                rTime = 12.0 - rTime;
            else
                rTime = -12.0 - rTime;
            
            if ( tmp_sTime <= midday )
                sTime = 12.0 - sTime;
            else
                sTime = -12.0 - sTime;
        } else {
            if ( ksp->transitAltitude( kdt, geo ).degree() > 0 ) {
                rTime = -24.0;
                sTime =  24.0;
            } else {
                rTime =  24.0;
                sTime = -24.0;
            }
        }
        
        tTime = tmp_tTime.secsTo( midday ) * 24.0 / 86400.0;
        if ( tmp_tTime <= midday )
            tTime = 12.0 - tTime;
        else
            tTime = -12.0 - tTime;
        
        float dy = kdt.date().daysInYear() - kdt.date().dayOfYear();
        vRise << QPointF( rTime, dy );
        vSet << QPointF( sTime, dy );
        vTransit << QPointF( tTime, dy );
    }    

    //Now, find continuous segments in each QVector and add each segment 
    //as a separate KPlotObject

    KPlotObject *oRise = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
    KPlotObject *oSet = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
    KPlotObject *oTransit = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
    bool needRiseLabel = true;
    bool needSetLabel = true;
    bool needTransertLabel = true;
    QString label;

    for ( int i=0; i<vRise.size(); ++i ) {
        /* if rise time is set under -23.0 or above 23.0, it means the planet never rise or set,
         * we add the current KPlotObject to CalendarView and create a new one. */
        if ( vRise.at( i ).x() > -23.0 && vRise.at( i ).x() < 23.0 ) {
            /* If the difference between to points is greater than 6 hours,
             * we consider the line continues on the other side of the view.
             * Add the current KPlotObject to CalendarView and create a new one. */
            if ( i > 0 && fabs( vRise.at( i ).x() - vRise.at( i-1 ).x() ) > 6.0 ) { 
                scUI->CalendarView->addPlotObject( oRise );
                oRise = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
                needRiseLabel = true;
            }
            // Set label if the line is on the night area, and if it needs one.
            if ( needRiseLabel 
                 && vRise.at(i).x() > scUI->CalendarView->getSetTime( i ) 
                 && vRise.at(i).x() < scUI->CalendarView->getRiseTime( i ) ) {
                label = i18nc( "A planet rises from the horizon", "%1 rises", ksp->name() );
                needRiseLabel = false;
            } else
                label = QString();
            /* When the line is over day area, we set needRiseLabel to true, so, the next
             * time the line pass over the night area, a label will be draw. */
            if ( vRise.at(i).x() >= scUI->CalendarView->getRiseTime( i ) )
                needRiseLabel = true;
            // Add the current point to KPlotObject
            oRise->addPoint( vRise.at(i), label );
        } else {
            scUI->CalendarView->addPlotObject( oRise );
            oRise = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
            needRiseLabel = true;
        }
        
        // Same process for set time
        if ( vSet.at( i ).x() > -23.0 && vSet.at( i ).x() < 23.0) {
            if ( i > 0 && fabs( vSet.at( i ).x() - vSet.at( i-1 ).x() ) > 6.0 ) {
                scUI->CalendarView->addPlotObject( oSet );
                oSet = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
                needSetLabel = true;
            }
            if ( needSetLabel && vSet.at(i).x() > scUI->CalendarView->getSetTime( i ) && vSet.at(i).x() < scUI->CalendarView->getRiseTime( i ) ){
                label = i18nc( "A planet sets from the horizon", "%1 sets", ksp->name() );
                needSetLabel = false;
            } else
                label  = QString();
            if ( vSet.at(i).x() <= scUI->CalendarView->getSetTime( i ) )
                needSetLabel = true;
            oSet->addPoint( vSet.at(i), label );
        } else {
            scUI->CalendarView->addPlotObject( oSet );
            oSet = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
            needSetLabel = true;
        }
        
        // Same process for transit time
        if ( vTransit.at( i ).x() > -23.0 && vTransit.at( i ).x() < 23.0) {
            if ( i > 0 && fabs( vTransit.at( i ).x() - vTransit.at( i-1 ).x() ) > 6.0 ) {
                scUI->CalendarView->addPlotObject( oTransit );
                oTransit = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
                needTransertLabel = true;
            }
            if ( needTransertLabel && vTransit.at(i).x() > scUI->CalendarView->getSetTime( i ) && vTransit.at(i).x() < scUI->CalendarView->getRiseTime( i ) ) {
                label = i18nc( "A planet transits across the meridian", "%1 transits", ksp->name() );
                needTransertLabel = false;
            } else
                label = QString();
            if ( vTransit.at(i).x() <= scUI->CalendarView->getSetTime( i ) || vTransit.at(i).x() >= scUI->CalendarView->getRiseTime( i ) )
                needTransertLabel = true;
            oTransit->addPoint( vTransit.at(i), label );
        } else {
            scUI->CalendarView->addPlotObject( oTransit );
            oTransit = new KPlotObject( pColor, KPlotObject::Lines, 2.0 );
            needTransertLabel = true;
        }
    }
    
    // Add the last points
    scUI->CalendarView->addPlotObject( oRise );
    scUI->CalendarView->addPlotObject( oSet );
    scUI->CalendarView->addPlotObject( oTransit );
}

void SkyCalendar::slotPrint() {
    QPainter p;                 // Our painter object
    QPrinter printer;           // Our printer object
    QString str_legend;         // Text legend
    QString str_year;           // Calendar's year
    int text_height = 200;      // Height of legend text zone in points
    QSize calendar_size;        // Initial calendar widget size
    QFont calendar_font;        // Initial calendar font
    int calendar_font_size;     // Initial calendar font size

    // Set printer resolution to 300 dpi
    printer.setResolution( 300 );

    // Open print dialog
    QPointer<QPrintDialog> dialog( KdePrint::createPrintDialog( &printer, this ) );
    dialog->setWindowTitle( i18n( "Print sky calendar" ) );
    if ( dialog->exec() == QDialog::Accepted ) {
        // Change mouse cursor
        QApplication::setOverrideCursor( Qt::WaitCursor );

        // Save calendar widget font
        calendar_font = scUI->CalendarView->font();
        // Save calendar widget font size
        calendar_font_size = calendar_font.pointSize();
        // Save calendar widget size
        calendar_size = scUI->CalendarView->size();

        // Set text legend
        str_year.setNum( year() );
        str_legend = i18n( "Sky Calendar" );
        str_legend += "\n";
        str_legend += geo->fullName();
        str_legend += " - ";
        str_legend += str_year;

        // Create a rectangle for legend text zone
        QRect text_rect( 0, 0, printer.width(), text_height );

        // Increase calendar widget font size so it looks good in 300 dpi
        calendar_font.setPointSize( calendar_font_size * 3 );
        scUI->CalendarView->setFont( calendar_font );
        // Increase calendar widget size to fit the entire page
        scUI->CalendarView->resize( printer.width(), printer.height() - text_height );

        // Create a pixmap and render calendar widget into it
        QPixmap pixmap(  scUI->CalendarView->size() );
        scUI->CalendarView->render( &pixmap );

        // Begin painting on printer
        p.begin( &printer );
        // Draw legend
        p.drawText( text_rect, Qt::AlignLeft, str_legend );
        // Draw calendar
        p.drawPixmap( 0, text_height, pixmap );
        // Ending painting
        p.end();

        // Restore calendar widget font size
        calendar_font.setPointSize( calendar_font_size );
        scUI->CalendarView->setFont( calendar_font );
        // Restore calendar widget size
        scUI->CalendarView->resize( calendar_size );

        // Restore mouse cursor
        QApplication::restoreOverrideCursor();
    }
    delete dialog;
}

void SkyCalendar::slotLocation() {
    QPointer<LocationDialog> ld = new LocationDialog( this );
    if ( ld->exec() == QDialog::Accepted ) {
        GeoLocation *newGeo = ld->selectedCity();
        if ( newGeo ) {
            geo = newGeo;
            scUI->LocationButton->setText( geo->fullName() );
        }
    }
    delete ld;
    
    scUI->CalendarView->setHorizon();
    slotFillCalendar();
}

GeoLocation* SkyCalendar::get_geo()
{
    return geo;
}

#include "skycalendar.moc"
