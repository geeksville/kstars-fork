
#ifndef SKYGUIDES_H_
#define SKYGUIDES_H_

#include <QWidget>
#include <QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QTextStream>

#include <KUrl>
#include <KFileDialog>

#include "skyguidesspace.h"
#include "ksutils.h"

class SkyGuidesSpace::SkyGuides : public QObject
{
    Q_OBJECT

public:
    SkyGuides(QWidget* parent=0,int parentWidth=0,int parentHeight=0);
    GuidesDocument* guidesdocument;
    GuidesListModel *guides ;
    SlidesListModel *slides;
    void reload();
    void resize(int,int);
    void deleteall();

public slots:
    void onguidesClicked(int index);
    void onBackButtonClicked();
    void onViewImagesClicked(int slideIndex,int imageIndex);
    void onAddNewGuidesClicked();
    void onCloseButtonClicked();


private:
    QObject *baseObject,*guidesListObj,*slidesListObj,*closeButtonObj,*baseObject2;
    QDeclarativeView *qmlview;
    QDeclarativeView *imageview;
    QDeclarativeContext *ctxt1;
    QDeclarativeContext *ctxt2;
    KUrl newguidesloc;
    QFile newguides;
    QList<Guide*> allguideslist;
    QTextStream out;

};

#endif
