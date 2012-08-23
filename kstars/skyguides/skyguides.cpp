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

    guidesdocument->readBegin(KStandardDirs::locate( "appdata" , "skyguides.xml" ));
    guides = new GuidesListModel(guidesdocument->m_Guides);

    qmlview = new QDeclarativeView(parent);
    imageview = new QDeclarativeView(parent);

    ctxt = qmlview->rootContext();
    ctxt->setContextProperty("feedModel",guides);
    ctxt->setContextProperty("slidesmodel",guides);

    //these are just for testing purposes
    ctxt->setContextProperty("thumbimage",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));
    ctxt->setContextProperty("trial_img1",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));
    ctxt->setContextProperty("trial_img2",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));
    ctxt->setContextProperty("trial_img3",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));


    ct = qmlview->rootContext();

    qmlview->setSource(QUrl(KStandardDirs::locate( "appdata" , "sgpanel.qml" )));
    imageview->setSource(QUrl(KStandardDirs::locate( "appdata" , "imagepanel.qml" )));

    imageview->hide();

    baseObject = qobject_cast<QObject *> (qmlview->rootObject());
    baseObject2 = qobject_cast<QObject *> (imageview->rootObject());

    guidesListObj = baseObject->findChild<QObject *>("guidesList");
    connect(guidesListObj, SIGNAL(guidesClicked(int)), this, SLOT(onguidesClicked(int)));
    connect(guidesListObj,SIGNAL(addNewGuidesClicked()),this,SLOT(onAddNewGuidesClicked()));
    connect(guidesListObj, SIGNAL(viewImagesClicked(int,int)), this, SLOT(onViewImagesClicked(int,int)));

    slidesListObj = baseObject->findChild<QObject *>("slidesList");
    connect(slidesListObj, SIGNAL(backButtonClicked(int)), this, SLOT(onBackButtonClicked(int)));

    closeButtonObj = baseObject2->findChild<QObject *>("closebutton");
    connect(closeButtonObj, SIGNAL(closeButtonClicked()), this, SLOT(onCloseButtonClicked()));

}

void SkyGuides::onguidesClicked(int index)
{

    slides = new SlidesListModel(guidesdocument->m_Guides);
    slides->currentIndex = (guides->rowCount()-1)-index;
    ctxt->setContextProperty("slidesmodel",slides);

}

void SkyGuides::onBackButtonClicked()
{
    delete slides;
}

void SkyGuides::onViewImagesClicked(int slideindex,int imageindex)
{
    imageview->show();
    imageview->move(500,200);
}
void SkyGuides::onCloseButtonClicked()
{
    imageview->close();
}

void SkyGuides::onAddNewGuidesClicked()
{
    newguidesloc = KFileDialog::getOpenUrl( QDir::homePath(), "*.xml" );

    if(!newguidesloc.isEmpty())
        qDebug()<<newguidesloc;

    QFile newfile (newguidesloc.path());
    newfile.open(QIODevice::ReadOnly);
    QFile guidesdocloc (KStandardDirs::locate( "appdata" , "skyguides.xml" ));

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
    guidesdocument->readBegin(KStandardDirs::locate( "appdata" , "skyguides.xml" ));

    guides = new GuidesListModel(guidesdocument->m_Guides);

    ctxt = qmlview->rootContext();
    ctxt->setContextProperty("feedModel",guides);
    ctxt->setContextProperty("slidesmodel",guides);

    qmlview->show();
}

