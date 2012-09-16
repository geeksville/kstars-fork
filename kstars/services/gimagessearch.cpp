#include "gimagessearch.h"

#include "skyobject.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

GImagesSearch::GImagesSearch(QNetworkAccessManager *manager, QObject *parent)
    : QObject(parent), m_NetworkManager(manager)
{
    connect(m_NetworkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    //connect(m_NetworkManager, )
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
}

void GImagesSearch::replyFinished(QNetworkReply *reply)
{
    QString pageData = reply->readAll();

    int index = pageData.indexOf("?imgurl=", 0);
    while(index >= 0) {
        index += 8; //move to end of "?imgurl=" marker

        //Image URL is everything from index to next occurrence of "&"
        QUrl url(pageData.mid(index, pageData.indexOf("&", index) - index));
        m_LastSearchResult.append(url);

        index = pageData.indexOf("?imgurl=", index);
    }

    foreach(QUrl url, m_LastSearchResult) {
        qDebug() << url.toString() << "\n";
    }
}
