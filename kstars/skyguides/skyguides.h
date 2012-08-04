
#include <QWidget>
#include <QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>
#include <QStringList>
#include <QDebug>
#include <QFile>

#include <KUrl>
#include <KFileDialog>

#include "skyguidesspace.h"

class SkyGuidesSpace::SkyGuides : public QObject
{
    Q_OBJECT

public:
    SkyGuides(QWidget* parent=0);
    GuidesDocument* guideslist;
    GuidesListModel *guides ;
    SlidesListModel *slides;
    void addNewGuides(QString &newgloc);

public slots:
    void onguidesClicked(int index);
    void onBackButtonClicked();
    void onViewImagesClicked(int slideIndex);
    void onAddNewClicked();

private:
    QObject *baseObject,*guidesListObj,*slidesListObj;
    QDeclarativeView *qmlview;
    QDeclarativeView *imageview;
    QDeclarativeContext *ctxt;
    QStringList lst;
    KUrl newguidesloc;
    QFile newguides;


};
