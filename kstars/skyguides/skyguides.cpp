#include "skyguides.h"
#include "guidesdocument.h"
#include "guideslistmodel.h"
#include "slideslistmodel.h"
#include "qglobal.h"
#include "kstandarddirs.h"

#include <QGraphicsObject>

using namespace SkyGuidesSpace;

SkyGuides::SkyGuides(QWidget *parent) : QObject(parent)
{

    guideslist = new GuidesDocument();
    guideslist->readBegin("/home/rmr/kstars/kstars/skyguides.xml");

    guides = new GuidesListModel(guideslist->m_Guides);

    qmlview = new QDeclarativeView(parent);
    ctxt = qmlview->rootContext();
    ctxt->setContextProperty("feedModel",guides);
    ctxt->setContextProperty("slidesmodel",guides);


    qmlview->setSource(QUrl("/home/rmr/kstars/kstars/sgpanel.qml"));
    baseObject = qobject_cast<QObject *> (qmlview->rootObject());

    guidesListObj = baseObject->findChild<QObject *>("guidesList");
    connect(guidesListObj, SIGNAL(guidesClicked(int)), this, SLOT(onguidesClicked(int)));
    connect(guidesListObj, SIGNAL(addNewClicked()), this, SLOT(onAddNewClicked()));
    slidesListObj = baseObject->findChild<QObject *>("slidesList");
    connect(slidesListObj, SIGNAL(backButtonClicked(int)), this, SLOT(onBackButtonClicked(int)));

}

void SkyGuides::onguidesClicked(int index)
{

    slides = new SlidesListModel(guideslist->m_Guides);
    slides->currentIndex = index;
    ctxt->setContextProperty("slidesmodel",slides);

}

void SkyGuides::onBackButtonClicked()
{
    delete slides;
}

void SkyGuides::onViewImagesClicked(int slideindex)
{

}
void SkyGuides::onAddNewClicked()
{
    newguidesloc = KFileDialog::getOpenUrl( QDir::homePath(), "*.xml" );

    if(!newguidesloc.isEmpty())
        qDebug()<<newguidesloc;



}
void SkyGuides::addNewGuides(QString &newgloc)
{



}
