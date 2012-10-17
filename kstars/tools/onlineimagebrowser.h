/***************************************************************************
                          onlineimagebrowser.h  -  K Desktop Planetarium
                             -------------------
    begin                : Mon Sept 10 2012
    copyright            : (C) 2012 by Lukasz Jaskiewicz and Rafal Kulaga
    email                : lucas.jaskiewicz@gmail.com, rl.kulaga@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ONLINEIMAGEBROWSER_H
#define ONLINEIMAGEBROWSER_H

#include "ui_onlineimagebrowser.h"

#include "KDialog"

#include <QFrame>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

class AstroBinApi;
class GImagesSearch;
class SkyObject;
class KJob;

namespace KIO {
    class StoredTransferJob;
}

class OnlineImageBrowser_ui :public QFrame, public Ui::OnlineImageBrowser
{
    Q_OBJECT

public:
    OnlineImageBrowser_ui(QWidget *parent = 0);
};

class OnlineImageBrowser : public KDialog
{
    Q_OBJECT

public:
    OnlineImageBrowser(SkyObject *obj, bool thumbnailPickMode = 0, QWidget *parent = false);
    ~OnlineImageBrowser();

    inline void setObject(SkyObject *obj) { m_Object = obj; }
    SkyObject* object() { return m_Object; }

private slots:
    void slotAstrobinSearch();
    void slotAstrobinSearchCompleted(bool ok);
    void slotGoogleSearch();
    void slotGoogleSearchCompleted(bool ok);

    void slotSetFromList(int row);

    void slotJobResult(KJob *job);

    void slotUnset();
    void slotSave();
    void slotEdit();

private:
    enum DATA_SOURCE
    {
        ASTROBIN,
        GOOGLE_IMAGES
    };

    void clearImagesList();
    void readExistingImagesForCurrentSearchType();
    void scaleAndAddPixmap(QPixmap *pixmap);
    void killAllRunningJobs();

    OnlineImageBrowser_ui *m_Ui;

    QNetworkAccessManager *m_NetworkManager;
    AstroBinApi *m_AstrobinApi;
    GImagesSearch *m_GImagesApi;

    bool m_ThumbnailPickMode;
    SkyObject *m_Object;

    DATA_SOURCE m_SearchType;
    QList<QPixmap*> m_AstrobinImages;
    QList<QPixmap*> m_GoogleImages;
    QList<KIO::StoredTransferJob*> m_Jobs;
};

#endif // ONLINEIMAGEBROWSER_H
