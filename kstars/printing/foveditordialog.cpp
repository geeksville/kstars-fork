/***************************************************************************
                          foveditordialog.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Fri Aug 12 2011
    copyright            : (C) 2011 by Rafał Kułaga
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

#include "foveditordialog.h"

#include <KIO/NetAccess>
#include <KTemporaryFile>
#include <KFileDialog>
#include <KMessageBox>
#include <KDebug>

#include "printingwizard.h"

FovEditorDialogUI::FovEditorDialogUI(QWidget *parent) : QFrame(parent)
{
    setupUi(this);

    setWindowTitle(i18n("Field of View Snapshot Browser"));
}

FovEditorDialog::FovEditorDialog(PrintingWizard *wizard, QWidget *parent) : KDialog(parent),
    m_ParentWizard(wizard), m_CurrentIndex(0)
{
    m_EditorUi = new FovEditorDialogUI(this);
    setMainWidget(m_EditorUi);
    setButtons(KDialog::Close);

    setupWidgets();
    setupConnections();
}

void FovEditorDialog::slotNextFov()
{
    slotSaveDescription();

    if(m_CurrentIndex < m_ParentWizard->getFovSnapshotList()->size() - 1)
    {
        m_CurrentIndex++;

        updateFovImage();
        updateButtons();
        updateDescriptions();
    }
}

void FovEditorDialog::slotPreviousFov()
{
    slotSaveDescription();

    if(m_CurrentIndex > 0)
    {
        m_CurrentIndex--;

        updateFovImage();
        updateButtons();
        updateDescriptions();
    }
}

void FovEditorDialog::slotCaptureAgain()
{
    hide();
    m_ParentWizard->recaptureFov(m_CurrentIndex);
}

void FovEditorDialog::slotDelete()
{
    if(m_CurrentIndex > m_ParentWizard->getFovSnapshotList()->size() - 1)
    {
        return;
    }

    delete m_ParentWizard->getFovSnapshotList()->at(m_CurrentIndex);
    m_ParentWizard->getFovSnapshotList()->removeAt(m_CurrentIndex);

    if(m_CurrentIndex == m_ParentWizard->getFovSnapshotList()->size())
    {
        m_CurrentIndex--;
    }

    updateFovImage();
    updateButtons();
    updateDescriptions();
}

void FovEditorDialog::slotSaveDescription()
{
    if(m_CurrentIndex < m_ParentWizard->getFovSnapshotList()->size())
    {
        m_ParentWizard->getFovSnapshotList()->at(m_CurrentIndex)->setDescription(m_EditorUi->descriptionEdit->text());
    }
}

void FovEditorDialog::slotSaveImage()
{
    if(m_CurrentIndex >= m_ParentWizard->getFovSnapshotList()->size())
    {
        return;
    }

    //If the filename string contains no "/" separators, assume the
    //user wanted to place a file in their home directory.
    QString url = KFileDialog::getSaveUrl(QDir::homePath(), "image/png image/jpeg image/gif image/x-portable-pixmap image/bmp").url();
    KUrl fileUrl;
    if(!url.contains("/"))
    {
        fileUrl = QDir::homePath() + '/' + url;
    }

    else
    {
        fileUrl = url;
    }

    KTemporaryFile tmpfile;
    tmpfile.open();
    QString fname;

    if(fileUrl.isValid())
    {
        if(fileUrl.isLocalFile())
        {
            fname = fileUrl.toLocalFile();
        }

        else
        {
            fname = tmpfile.fileName();
        }

        //Determine desired image format from filename extension
        QString ext = fname.mid(fname.lastIndexOf(".") + 1);
        // export as raster graphics
        const char* format = "PNG";

        if(ext.toLower() == "png") {format = "PNG";}
        else if(ext.toLower() == "jpg" || ext.toLower() == "jpeg" ) {format = "JPG";}
        else if(ext.toLower() == "gif") {format = "GIF";}
        else if(ext.toLower() == "pnm") {format = "PNM";}
        else if(ext.toLower() == "bmp") {format = "BMP";}
        else
        {
            kWarning() << i18n("Could not parse image format of %1; assuming PNG.", fname);
        }

        if(!m_ParentWizard->getFovSnapshotList()->at(m_CurrentIndex)->getPixmap().save(fname, format))
        {
            kDebug() << i18n("Error: Unable to save image: %1 ", fname);
        }

        else
        {
            kDebug() << i18n("Image saved to file: %1", fname);
        }
    }

    if(tmpfile.fileName() == fname)
    {
        //attempt to upload image to remote location
        if(!KIO::NetAccess::upload(tmpfile.fileName(), fileUrl, this))
        {
            QString message = i18n( "Could not upload image to remote location: %1", fileUrl.prettyUrl() );
            KMessageBox::sorry( 0, message, i18n( "Could not upload file" ) );
        }
    }
}

void FovEditorDialog::setupWidgets()
{
    if(m_ParentWizard->getFovSnapshotList()->size() > 0)
    {
        m_EditorUi->imageLabel->setPixmap(m_ParentWizard->getFovSnapshotList()->first()->getPixmap());
    }

    updateButtons();
    updateDescriptions();
}

void FovEditorDialog::setupConnections()
{
    connect(m_EditorUi->previousButton, SIGNAL(clicked()), this, SLOT(slotPreviousFov()));
    connect(m_EditorUi->nextButton, SIGNAL(clicked()), this, SLOT(slotNextFov()));
    connect(m_EditorUi->recaptureButton, SIGNAL(clicked()), this, SLOT(slotCaptureAgain()));
    connect(m_EditorUi->deleteButton, SIGNAL(clicked()), this, SLOT(slotDelete()));
    connect(m_EditorUi->descriptionEdit, SIGNAL(editingFinished()), this, SLOT(slotSaveDescription()));
    connect(m_EditorUi->saveButton, SIGNAL(clicked()), this, SLOT(slotSaveImage()));
}

void FovEditorDialog::updateButtons()
{
    m_EditorUi->previousButton->setEnabled(m_CurrentIndex > 0);
    m_EditorUi->nextButton->setEnabled(m_CurrentIndex < m_ParentWizard->getFovSnapshotList()->size() - 1);
}

void FovEditorDialog::updateDescriptions()
{
    if(m_ParentWizard->getFovSnapshotList()->size() == 0)
    {
        m_EditorUi->imageLabel->setText("No captured field of view images.");
        m_EditorUi->fovInfoLabel->setText(QString());
        m_EditorUi->recaptureButton->setEnabled(false);
        m_EditorUi->deleteButton->setEnabled(false);
        m_EditorUi->descriptionEdit->setEnabled(false);
        m_EditorUi->saveButton->setEnabled(false);
    }

    else
    {
        FOV *fov = m_ParentWizard->getFovSnapshotList()->at(m_CurrentIndex)->getFov();

        QString fovDescription = i18n("FOV (%1/%2): %3 (%4' x %5')",
                QString::number(m_CurrentIndex + 1),
                QString::number(m_ParentWizard->getFovSnapshotList()->size()),
                fov->name(),
                QString::number(fov->sizeX()),
                QString::number(fov->sizeY()));

        m_EditorUi->fovInfoLabel->setText(fovDescription);

        m_EditorUi->descriptionEdit->setText(m_ParentWizard->getFovSnapshotList()->at(m_CurrentIndex)->getDescription());
    }
}

void FovEditorDialog::updateFovImage()
{
    if(m_CurrentIndex < m_ParentWizard->getFovSnapshotList()->size())
    {
        m_EditorUi->imageLabel->setPixmap(m_ParentWizard->getFovSnapshotList()->at(m_CurrentIndex)->getPixmap());
    }
}
