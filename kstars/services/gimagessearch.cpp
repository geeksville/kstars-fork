/***************************************************************************
                          gimagessearch.cpp  -  K Desktop Planetarium
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

#include "gimagessearch.h"

#include "skyobject.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

GImagesSearch::GImagesSearch(QNetworkAccessManager *manager, QObject *parent)
    : QObject(parent), m_NetworkManager(manager)
{
    connect(m_NetworkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
}

void GImagesSearch::searchObjectImages(const SkyObject *object)
{
    QUrl requestUrl("http://images.google.com/images");
    //Search for the primary name, or longname and primary name
    QString sName = QString("\"%1\"").arg(object->name());
    if(object->longname() != object->name()) {
        sName = QString("\"%1\" ").arg(object->longname()) + sName;
    }
    requestUrl.addQueryItem("q", sName); //add the Google-image query string

    QNetworkRequest request(requestUrl);
    request.setOriginatingObject(this);
    m_NetworkManager->get(request);
}

void GImagesSearch::replyFinished(QNetworkReply *reply)
{
    if(reply->request().originatingObject() != static_cast<QObject*>(this)) {
        return;
    }

    m_LastSearchResult.clear();

    QString pageData = reply->readAll();

    int index = pageData.indexOf("?imgurl=", 0);
    while(index >= 0) {
        index += 8; //move to end of "?imgurl=" marker

        //Image URL is everything from index to next occurrence of "&"
        QUrl url(pageData.mid(index, pageData.indexOf("&", index) - index));
        m_LastSearchResult.append(url);

        index = pageData.indexOf("?imgurl=", index);
    }

    emit searchFinished(true);
}
