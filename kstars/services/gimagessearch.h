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
