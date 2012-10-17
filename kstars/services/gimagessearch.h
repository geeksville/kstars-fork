/***************************************************************************
                          gimagessearch.h  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Sept 7 2012
    copyright            : (C) 2012 by Lukasz Jaskiewicz
    email                : lucas.jaskiewicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GIMAGESSEARCH_H
#define GIMAGESSEARCH_H

#include <QObject>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

class SkyObject;

class GImagesSearch : public QObject
{
    Q_OBJECT

public:
    GImagesSearch(QNetworkAccessManager *manager, QObject *parent = 0);

    void searchObjectImages(const SkyObject *object);

    QList<QUrl> getResult() { return m_LastSearchResult; }

signals:
    void searchFinished(bool resultOK);

private slots:
    void replyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_NetworkManager;
    QList<QUrl> m_LastSearchResult;
};

#endif // GIMAGESSEARCH_H
