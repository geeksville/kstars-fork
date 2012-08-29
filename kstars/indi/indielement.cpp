/*  INDI Element
    Copyright (C) 2003 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    2004-01-15	INDI element is the most basic unit of the INDI KStars client.
 */

#include <libindi/base64.h>

#include "indielement.h"
#include "indiproperty.h"
#include "indigroup.h"
#include "indidevice.h"

#include <indicom.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QSlider>
#include <QDir>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QFont>
#include <QDoubleSpinBox>
#include <QDebug>

#include <KPushButton>
#include <KSqueezedTextLabel>
#include <KLineEdit>
#include <KLocale>
#include <KLed>
#include <KFileDialog>
#include <KMessageBox>

/*******************************************************************
** INDI Element
*******************************************************************/
INDI_E::INDI_E(INDI_P *gProp, INDI::Property *dProp)
{
  guiProp  = gProp;
  dataProp = dProp;

  EHBox     = new QHBoxLayout;
  EHBox->setMargin(0);
  EHBox->setSpacing(KDialog::spacingHint());

  tp        = NULL;
  sp        = NULL;
  np        = NULL;
  label_w   = NULL;
  read_w    = NULL;
  write_w   = NULL;
  spin_w    = NULL;
  slider_w  = NULL;
  push_w    = NULL;
  browse_w  = NULL;
  check_w   = NULL;
  led_w     = NULL;
  hSpacer   = NULL;

}

INDI_E::~INDI_E()
{
    delete (EHBox);
    delete (label_w);
    delete (read_w);
    delete (write_w);
    delete (spin_w);
    delete (slider_w);
    delete (push_w);
    delete (browse_w);
    delete (check_w);
    delete (led_w);
    delete (hSpacer);
}

void INDI_E::buildSwitch(QButtonGroup* groupB, ISwitch *sw)
{
    name  = QString(sw->name);
    label = QString(sw->label);

    sp = sw;

    if (label.isEmpty())
        label = name;

    if (groupB == NULL)
        return;

    switch (guiProp->getGUIType())
    {
    case PG_BUTTONS:
        push_w = new KPushButton(label, guiProp->getGroup()->getContainer());
        groupB->addButton(push_w);

        syncSwitch();

        guiProp->addWidget(push_w);

        push_w->show();

        break;

    case PG_RADIO:
        check_w = new QCheckBox(label, guiProp->getGroup()->getContainer());
        groupB->addButton(check_w);

        syncSwitch();

        guiProp->addWidget(check_w);

        check_w->show();

        break;

    default:
        break;

    }

}

void INDI_E::buildMenuItem(ISwitch *sw)
{
    buildSwitch(NULL, sw);
}

void INDI_E::buildText(IText *itp)
{
    name  = QString(itp->name);
    label = QString(itp->label);

    tp = itp;

    if (label.isEmpty())
        label = name;

    setupElementLabel();

    text = QString(tp->text);

    switch (dataProp->getPermission())
    {
    case IP_RW:
        setupElementRead(ELEMENT_READ_WIDTH);
        setupElementWrite(ELEMENT_WRITE_WIDTH);

        break;

    case IP_RO:
        setupElementRead(ELEMENT_FULL_WIDTH);
        break;

    case IP_WO:
        setupElementWrite(ELEMENT_FULL_WIDTH);
        break;
    }


    guiProp->addLayout(EHBox);
    //pp->PVBox->addLayout(EHBox);
}

void INDI_E::setupElementLabel()
{
    QPalette palette;

    label_w = new KSqueezedTextLabel(guiProp->getGroup()->getContainer());
    label_w->setMinimumWidth(ELEMENT_LABEL_WIDTH);
    label_w->setMaximumWidth(ELEMENT_LABEL_WIDTH);
    label_w->setFrameShape( KSqueezedTextLabel::Box );

    palette.setColor(label_w->backgroundRole(),  QColor( 224, 232, 238 ));
    label_w->setPalette(palette);
    label_w->setTextFormat( Qt::RichText );
    label_w->setAlignment( Qt::AlignCenter );
    label_w->setWordWrap(true);

    if (label.length() > MAX_LABEL_LENGTH)
    {
        QFont tempFont(  label_w->font() );
        tempFont.setPointSize( tempFont.pointSize() - MED_INDI_FONT );
        label_w->setFont( tempFont );
    }

    label_w->setText(label);

    EHBox->addWidget(label_w);
}

void INDI_E::syncSwitch()
{
    QFont buttonFont;

    switch (guiProp->getGUIType())
    {
       case PG_BUTTONS:
        if (sp->s == ISS_ON)
        {
            push_w->setDown(true);
            buttonFont = push_w->font();
            buttonFont.setBold(true);
            push_w->setFont(buttonFont);
        }
        else
        {
            push_w->setDown(false);
            buttonFont = push_w->font();
            buttonFont.setBold(false);
            push_w->setFont(buttonFont);
        }
        break;

    default:
        break;

    }
}

void INDI_E::syncText()
{

    if (tp == NULL)
        return;

    read_w->setText(tp->text);

}

void INDI_E::syncNumber()
{

    char iNumber[32];
    if (np == NULL || read_w == NULL)
        return;

    numberFormat(iNumber, np->format, np->value);

    text = iNumber;

    read_w->setText(text);

    if (spin_w)
    {
        if (np->min != spin_w->minimum())
            setMin();
        if (np->max != spin_w->maximum())
            setMax();
    }

}

void INDI_E::updateTP()
{

    if (tp == NULL)
        return;

    IUSaveText(tp, write_w->text().toLatin1().constData());
}

void INDI_E::updateNP()
{
    if (np == NULL)
        return;

    if (write_w != NULL)
    {
        if (write_w->text().isEmpty())
            return;

        f_scansexa(write_w->text().toLatin1().constData(), &(np->value));
        return;
    }

    if (spin_w != NULL)
        np->value = spin_w->value();

}

void INDI_E::setText(const QString &newText)
{
    if (tp == NULL)
        return;

   switch(dataProp->getPermission())
   {
      case IP_RO:
       read_w->setText(newText);
       break;

      case IP_WO:
      case IP_RW:
       text = newText;
       IUSaveText(tp, newText.toLatin1().constData());
       read_w->setText(newText);
       write_w->setText(newText);
        break;
   }

}

void INDI_E::buildBLOB(IBLOB *ibp)
{

    name  = QString(ibp->name);
    label = QString(ibp->label);

    bp = ibp;

    if (label.isEmpty())
        label = name;

    setupElementLabel();

    text = i18n("INDI DATA STREAM");

    switch (dataProp->getPermission())
    {
    case IP_RW:
        setupElementRead(ELEMENT_READ_WIDTH);
        setupElementWrite(ELEMENT_WRITE_WIDTH);
        setupBrowseButton();
        break;

    case IP_RO:
        setupElementRead(ELEMENT_FULL_WIDTH);
        break;

    case IP_WO:
        setupElementWrite(ELEMENT_FULL_WIDTH);
        setupBrowseButton();
        break;
    }

    guiProp->addLayout(EHBox);

}

void INDI_E::buildNumber  (INumber *inp)
{
    bool scale = false;
    char iNumber[32];

    name  = QString(inp->name);
    label = QString(inp->label);

    np = inp;

    if (label.isEmpty())
        label = name;

    numberFormat(iNumber, np->format, np->value);
    text = iNumber;

    setupElementLabel();

    if (np->step != 0 && (np->max - np->min)/np->step <= 100)
        scale = true;

    switch (dataProp->getPermission())
    {
    case IP_RW:
        setupElementRead(ELEMENT_READ_WIDTH);
        if (scale)
            setupElementScale(ELEMENT_WRITE_WIDTH);
        else
            setupElementWrite(ELEMENT_WRITE_WIDTH);

        guiProp->addLayout(EHBox);
        break;

    case IP_RO:
        setupElementRead(ELEMENT_READ_WIDTH);
        guiProp->addLayout(EHBox);
        break;

    case IP_WO:
        if (scale)
            setupElementScale(ELEMENT_FULL_WIDTH);
        else
            setupElementWrite(ELEMENT_FULL_WIDTH);

        guiProp->addLayout(EHBox);

        break;
    }

}

void INDI_E::buildLight(ILight *ilp)
{

    name  = QString(ilp->name);
    label = QString(ilp->label);

    lp = ilp;

    if (label.isEmpty())
        label = name;

    led_w = new KLed (guiProp->getGroup()->getContainer());
    led_w->setMaximumSize(16,16);
    led_w->setLook( KLed::Sunken );

    syncLight();

    EHBox->addWidget(led_w);

    setupElementLabel();

    guiProp->addLayout(EHBox);

}

void INDI_E::syncLight()
{
    if (lp == NULL)
        return;

    switch (lp->s)
    {
    case IPS_IDLE:
        led_w->setColor(Qt::gray);
        break;

    case IPS_OK:
        led_w->setColor(Qt::green);
        break;

    case IPS_BUSY:
        led_w->setColor(Qt::yellow);
        break;

    case IPS_ALERT:
        led_w->setColor(Qt::red);
        break;

    default:
        break;

    }
}

void INDI_E::setupElementScale(int length)
{

    if (np == NULL)
        return;

    int steps = (int) ((np->max - np->min) / np->step);
    spin_w    = new QDoubleSpinBox(guiProp->getGroup()->getContainer());
    spin_w->setRange(np->min, np->max);
    spin_w->setSingleStep(np->step);
    spin_w->setValue(np->value);
    spin_w->setDecimals(2);

    slider_w  = new QSlider( Qt::Horizontal, guiProp->getGroup()->getContainer() );
    slider_w->setRange(0, steps);
    slider_w->setPageStep(1);
    slider_w->setValue((int) ((np->value - np->min) / np->step));

    connect(spin_w, SIGNAL(valueChanged(double)), this, SLOT(spinChanged(double )));
    connect(slider_w, SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int )));


    if (length == ELEMENT_FULL_WIDTH)
        spin_w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    else
        spin_w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    spin_w->setMinimumWidth( (int) (length * 0.45) );
    slider_w->setMinimumWidth( (int) (length * 0.55) );

    EHBox->addWidget(slider_w);
    EHBox->addWidget(spin_w);

}

void INDI_E::spinChanged(double value)
{

    int slider_value = (int) ((value - np->min) / np->step);
    slider_w->setValue(slider_value);

}

void INDI_E::sliderChanged(int value)
{

    double spin_value = (value * np->step) + np->min;
    spin_w->setValue(spin_value);


}

void INDI_E::setMin ()
{

    if (spin_w)
    {
        spin_w->setMinimum(np->min);
        spin_w->setValue(np->value);
    }
    if (slider_w)
    {
        slider_w->setMaximum((int) ((np->max - np->min) / np->step));
        slider_w->setMinimum(0);
        slider_w->setPageStep(1);
        slider_w->setValue( (int) ((np->value - np->min) / np->step ));
    }

}

void INDI_E::setMax ()
{
    if (spin_w)
    {
        spin_w->setMaximum(np->max);
        spin_w->setValue(np->value);
    }
    if (slider_w)
    {
        slider_w->setMaximum((int) ((np->max - np->min) / np->step));
        slider_w->setMinimum(0);
        slider_w->setPageStep(1);
        slider_w->setValue( (int) ((np->value - np->min) / np->step ));
    }
}

void INDI_E::setupElementWrite(int length)
{

    write_w = new KLineEdit( guiProp->getGroup()->getContainer());
    write_w->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred);
    write_w->setMinimumWidth( length );
    write_w->setMaximumWidth( length);

    write_w->setText(text);

    //QObject::connect(write_w, SIGNAL(returnPressed(QString)), this, SLOT(updateTP()));
    QObject::connect(write_w, SIGNAL(returnPressed()), guiProp, SLOT(sendText()));
    EHBox->addWidget(write_w);

}


void INDI_E::setupElementRead(int length)
{


    read_w = new KLineEdit( guiProp->getGroup()->getContainer() );
    read_w->setMinimumWidth( length );
    read_w->setFocusPolicy( Qt::NoFocus );
    read_w->setCursorPosition( 0 );
    read_w->setAlignment( Qt::AlignCenter );
    read_w->setReadOnly( true );
    read_w->setText(text);

    EHBox->addWidget(read_w);


}

void INDI_E::setupBrowseButton()
{
    browse_w = new KPushButton("...", guiProp->getGroup()->getContainer());
    browse_w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    browse_w->setMinimumWidth( MIN_SET_WIDTH );
    browse_w->setMaximumWidth( MAX_SET_WIDTH );

    EHBox->addWidget(browse_w);
    QObject::connect(browse_w, SIGNAL(clicked()), this, SLOT(browseBlob()));
}


void INDI_E::browseBlob()
{

    QFile fp;
    QString filename;
    QString format;
    QDataStream binaryStream;
    int data64_size=0, pos=0;
    unsigned char *data_file;
    KUrl currentURL;

    currentURL = KFileDialog::getOpenUrl( QDir::homePath(), "*");

    // if user presses cancel
    if (currentURL.isEmpty())
        return;

    if ( currentURL.isValid() )
        write_w->setText(currentURL.path());

    fp.setFileName(currentURL.path());

    if ( (pos = filename.lastIndexOf(".")) != -1)
        format = filename.mid (pos, filename.length());

    //qDebug() << "Filename is " << fp.fileName() << endl;

    if (!fp.open(QIODevice::ReadOnly))
    {
        KMessageBox::error(0, i18n("Cannot open file %1 for reading", filename));
        return;
    }

    binaryStream.setDevice(&fp);

    data_file = new unsigned char[fp.size()];

    bp->bloblen = fp.size();

    if (data_file == NULL)
    {
        KMessageBox::error(0, i18n("Not enough memory to load %1", filename));
        fp.close();
        return;
    }

    binaryStream.readRawData((char*)data_file, fp.size());

    bp->blob = new unsigned char[4*fp.size()/3+4];
    if (bp->blob == NULL)
    {
        KMessageBox::error(0, i18n("Not enough memory to convert file %1 to base64", filename));
        fp.close();
    }

    data64_size = to64frombits ( ((unsigned char *) bp->blob), data_file, fp.size());

    delete [] data_file;

    bp->size = data64_size;

    //qDebug() << "BLOB " << bp->name << " has size of " << bp->size << " and bloblen of " << bp->bloblen << endl;

    blobDirty = true;

}

const QString & INDI_E::getWriteField()
{
    if (write_w)
        return write_w->text();
    else
        return NULL;
}

const QString & INDI_E::getReadField()
{
    if (read_w)
        return read_w->text();
    else
        return NULL;
}

#include "indielement.moc"
