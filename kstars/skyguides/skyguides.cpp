#include "skyguides.h"
#include "guidesdocument.h"
#include "guideslistmodel.h"
#include "slideslistmodel.h"
#include "qglobal.h"
#include "kstandarddirs.h"
#include "guide.h"

#include <QGraphicsObject>

using namespace SkyGuidesSpace;

SkyGuides::SkyGuides(QWidget *parent) : QObject(parent)
{

    guidesdocument = new GuidesDocument();

    guideslocations.append("/home/rmr/kstars/kstars/data/skyguides.xml");
    for(int i=0;i<guideslocations.count();i++)
    {
        guidesdocument->readBegin(guideslocations[i]);
        allguideslist.append(guidesdocument->m_Guides);

    }
 //   guidesdocument->readBegin("/home/rmr/kstars/kstars/data/skyguides.xml");
    guides = new GuidesListModel(allguideslist);

    qmlview = new QDeclarativeView(parent);
    imageview = new QDeclarativeView(parent);

    ctxt = qmlview->rootContext();
    ctxt->setContextProperty("feedModel",guides);
    ctxt->setContextProperty("slidesmodel",guides);


    ct = qmlview->rootContext();
    ct->setContextProperty("pheight",parent->height());
    ct->setContextProperty("pwidth",parent->width());

    qmlview->setSource(QUrl("/home/rmr/Documents/untitled6/untitled6.qml"));
    imageview->setSource(QUrl("/home/rmr/Documents/untitled8/untitled8.qml"));

    imageview->hide();
    qDebug()<<"hai"<<KStandardDirs::locate("appdata","skyguides/sgpanel1.qml");
    //qDebug()<<"hai"<<KStandardDirs::locate("data","kstars/data/sgpanel1.qml");
    //qDebug()<<"hai"<<KStandardDirs::locate("appdata","skyguides.xml");


    //qmlview->setSource(KStandardDirs::locate("appdata","skyguides/sgpanel1.qml"));
    baseObject = qobject_cast<QObject *> (qmlview->rootObject());

    guidesListObj = baseObject->findChild<QObject *>("guidesList");
    connect(guidesListObj, SIGNAL(guidesClicked(int)), this, SLOT(onguidesClicked(int)));
    connect(guidesListObj,SIGNAL(addNewGuidesClicked()),this,SLOT(onAddNewGuidesClicked()));
    slidesListObj = baseObject->findChild<QObject *>("slidesList");
    connect(slidesListObj, SIGNAL(backButtonClicked(int)), this, SLOT(onBackButtonClicked(int)));
    connect(guidesListObj, SIGNAL(viewImagesClicked(int,int)), this, SLOT(onViewImagesClicked(int,int)));
}

void SkyGuides::onguidesClicked(int index)
{

    slides = new SlidesListModel(allguideslist);
    slides->currentIndex = (guides->rowCount()-1)-index;
    ctxt->setContextProperty("slidesmodel",slides);

}

void SkyGuides::onBackButtonClicked()
{
    delete slides;
}

void SkyGuides::onViewImagesClicked(int slideindex,int imageindex)
{
    qDebug()<<"index = "<<slideindex;

    imageview->show();
    imageview->move(500,500);



}
void SkyGuides::onAddNewGuidesClicked()
{
    newguidesloc = KFileDialog::getOpenUrl( QDir::homePath(), "*.xml" );

    if(!newguidesloc.isEmpty())
        qDebug()<<newguidesloc;

    QFile newfile (newguidesloc.path());
    newfile.open(QIODevice::ReadOnly);
    QFile guidesdocloc ("/home/rmr/kstars/kstars/data/skyguides.xml");

    QString line1;
    QString line2;
    qint64 curpos1;
    qint64 curpos2;
    bool loop=true;

    while(loop==true)
    {
        curpos2 = newfile.pos();
        line1 = newfile.readLine();
        if(line1.contains("<guide>"))
            break;
    }

    guidesdocloc.open(QIODevice::ReadWrite);

    while (loop==true)
    {
        curpos1 = guidesdocloc.pos();
        line2 = guidesdocloc.readLine();
        if(line2.contains("</guides>"))
            break;
    }

    out.setDevice(&guidesdocloc);
    guidesdocloc.seek(curpos1);
    newfile.seek(curpos2);

    do
    {
        line1 = newfile.readLine();
        out<<line1;
    }
    while (!line1.contains("</guides>"));

    guidesdocloc.close();
    newfile.close();

    reload();

}

void SkyGuides::reload()
{
    qmlview->close();

    if(guidesdocument)
        delete guidesdocument;

    if(guides)
        delete guides;

    if(slides)
        delete slides;

    guidesdocument = new GuidesDocument();
    guidesdocument->readBegin("/home/rmr/kstars/kstars/data/skyguides.xml");

    guides = new GuidesListModel(guidesdocument->m_Guides);

    ctxt = qmlview->rootContext();
    ctxt->setContextProperty("feedModel",guides);
    ctxt->setContextProperty("slidesmodel",guides);


    qmlview->show();
}

