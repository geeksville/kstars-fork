
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

class SkyGuidesSpace::SkyGuides : public QObject
{
    Q_OBJECT

public:
    SkyGuides(QWidget* parent=0);
    GuidesDocument* guidesdocument;
    GuidesDocument* guidesdocument1;
    GuidesListModel *guides ;
    SlidesListModel *slides;
    void reload();
    void deleteall();

public slots:
    void onguidesClicked(int index);
    void onBackButtonClicked();
    void onViewImagesClicked(int slideIndex,int imageIndex);
    void onAddNewGuidesClicked();

private:
    QObject *baseObject,*guidesListObj,*slidesListObj;
    QDeclarativeView *qmlview;
    QDeclarativeView *imageview;
    QDeclarativeContext *ctxt;
    QDeclarativeContext *ct;
    QStringList lst;
    KUrl newguidesloc;
    QFile newguides;
    QList<QString> guideslocations;
    QList<Guide*> allguideslist;
    QTextStream out;

};
