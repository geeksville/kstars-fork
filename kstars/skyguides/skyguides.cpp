#include "skyguides.h"
#include "guidesdocument.h"
#include "guideslistmodel.h"
#include "slideslistmodel.h"
#include "imageslistmodel.h"
#include "qglobal.h"
#include "kstandarddirs.h"
#include "guide.h"
#include "slide.h"
#include "image.h"

#include <QGraphicsObject>

using namespace SkyGuidesSpace;

SkyGuides::SkyGuides(QWidget *parent,int parentWidth,int parentHeight) : QObject(parent)
{

    currentGuideIndex=0;
    currentSlideIndex=0;

    KStandardDirs ksd;
    guidesdocument = new GuidesDocument();

    guidesdocument->readBegin(KStandardDirs::locate( "appdata" , "skyguides.xml" ));
//    guidesdocument->readBegin("/home/rmr/skyguidetrial.xml");
    guides = new GuidesListModel(guidesdocument->m_Guides);

    qmlview = new QDeclarativeView(parent);
    imageview = new QDeclarativeView(parent);

    qmlview->viewport()->setAutoFillBackground(false);
    qmlview->setAttribute(Qt::WA_TranslucentBackground);

    ctxt1 = qmlview->rootContext();
    ctxt1->setContextProperty("feedModel",guides);

    noOfSlides = guidesdocument->m_Guides[currentGuideIndex]->m_Slides.count();
    showSlide(guidesdocument->m_Guides[currentGuideIndex]->m_Slides[currentSlideIndex]);


   // ctxt1->setContextProperty("slidesmodel",guides);
    ctxt1->setContextProperty("w",(parentWidth*0.22));
    ctxt1->setContextProperty("h",(parentHeight*0.88));

    ctxt1->setContextProperty("background",KStandardDirs::locate( "appdata" , "skyguides_bg.png"));
    ctxt1->setContextProperty("sgsideselect",KStandardDirs::locate( "appdata" , "skyguides_side.png"));
  //  ctxt1->setContextProperty("background","/home/rmr/kde/src/kstars/kstars/data/skyguides_bg.png");
  //  ctxt1->setContextProperty("sgsideselect","/home/rmr/kde/src/kstars/kstars/data/skyguides_side.png");

    ctxt2 = imageview->rootContext();

    ctxt2->setContextProperty("trialImage",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));
 //   ctxt2->setContextProperty("trialImage","/home/rmr/kde/src/kstars/kstars/data/trial_img.jpeg");
    //these are just for testing purposes

//     ctxt1->setContextProperty("thumbimage",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));
//     ctxt1->setContextProperty("trial_img1",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));
//     ctxt1->setContextProperty("trial_img2",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));
//     ctxt1->setContextProperty("trial_img3",KStandardDirs::locate( "appdata" , "trial_img.jpeg"));

// ctxt1->setContextProperty("trial_img1","/home/rmr/kde/src/kstars/kstars/data/trial_img.jpeg");
// ctxt1->setContextProperty("trial_img2","/home/rmr/kde/src/kstars/kstars/data/trial_img.jpeg");
// ctxt1->setContextProperty("trial_img3","/home/rmr/kde/src/kstars/kstars/data/trial_img.jpeg");



   // FileName = KStandardDirs::locate("data","kstars/sgpanel.qml");
    qmlview->setSource(QUrl(KStandardDirs::locate( "appdata" , "sgpanel.qml" )));
//  qmlview->setSource(QUrl("/home/rmr/kde/src/kstars/kstars/data/sgpanel.qml"));

    imageview->setSource(QUrl(KStandardDirs::locate( "appdata" , "imagepanel.qml" )));
//imageview->setSource(QUrl("/home/rmr/kde/src/kstars/kstars/data/imagepanel.qml"));
    imageview->hide();

    baseObject = qobject_cast<QObject *> (qmlview->rootObject());
    baseObject2 = qobject_cast<QObject *> (imageview->rootObject());

    guidesListObj = baseObject->findChild<QObject *>("guidesList");
    connect(guidesListObj, SIGNAL(guidesClicked(int)), this, SLOT(onguidesClicked(int)));
    connect(guidesListObj,SIGNAL(addNewGuidesClicked()),this,SLOT(onAddNewGuidesClicked()));
    connect(guidesListObj,SIGNAL(nextSlideClicked()),this,SLOT(onNextSlideClicked()));
    connect(guidesListObj,SIGNAL(previousSlideClicked()),this,SLOT(onPreviousSlideClicked()));

     connect(guidesListObj, SIGNAL(viewImagesClicked(int)), this, SLOT(onViewImagesClicked(int)));

    closeButtonObj = baseObject2->findChild<QObject *>("closebutton");
    connect(closeButtonObj, SIGNAL(closeButtonClicked()), this, SLOT(onCloseButtonClicked()));


}

void SkyGuides::onguidesClicked(int index)
{

    //slides = new SlidesListModel(guidesdocument->m_Guides);
    //slides->currentIndex = (guides->rowCount()-1)-index;
    //ctxt1->setContextProperty("slidesmodel",slides);
    currentGuideIndex = index;
    currentSlideIndex = 0;
    showSlide(guidesdocument->m_Guides[currentGuideIndex]->m_Slides[currentSlideIndex]);

}

void SkyGuides::onNextSlideClicked()
{
    if((currentSlideIndex)<noOfSlides)
    {
        showSlide(guidesdocument->m_Guides[currentGuideIndex]->m_Slides[currentSlideIndex++]);
        qDebug()<<"slideindex = "<<currentSlideIndex;
    }
}

void SkyGuides::onPreviousSlideClicked()
{
    if((currentSlideIndex)>0)
    {
        qDebug()<<"slideindex = "<<currentSlideIndex;
        showSlide(guidesdocument->m_Guides[currentGuideIndex]->m_Slides[--currentSlideIndex]);
    }

}

void SkyGuides::showSlide(Slide *slide)
{
    ctxt1->setContextProperty("slidetext",slide->text());
    images = new ImagesListModel(slide);
    ctxt1->setContextProperty("imageModel",images);
}

void SkyGuides::showSlideImages(Slide* slide)
{
    images = new ImagesListModel(slide);
    ctxt1->setContextProperty("imageModel",images);
}

//void SkyGuides::onBackButtonClicked()
//{
//    delete slides;
//}

void SkyGuides::onViewImagesClicked(int imageindex)
{
    if(imageviewer)
     {
        delete imageviewer;
        imageviewer = 0;
      }
    if(qimage)
     {
        delete qimage;
        qimage = 0;
      }

    currentImage = guidesdocument->m_Guides[currentGuideIndex]->m_Slides[currentSlideIndex]->m_Images[imageindex];
    //qimage = new QImage(currentImage->url());
    imageviewer = new ImageViewer(currentImage->url());
//    imageviewer->setMinimumHeight(qimage->height());
//    imageviewer->setMinimumWidth(qimage->width());
    imageviewer->show();

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
    QFile guidesdocloc;

    QString FileName = KStandardDirs::locate( "appdata", "skyguides.xml" );
 //   QString FileName = "/home/rmr/kde/src/kstars/kstars/data/skyguides.xml";

    guidesdocloc.setFileName(FileName);

  //  guidesdocloc.setFileName("/home/rmr/kstars/kstars/data/skyguides.xml");

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
    QString FileName = KStandardDirs::locate( "appdata", "skyguides.xml" );
//    QString FileName = "/home/rmr/kde/src/kstars/kstars/data/skyguides.xml";


    guidesdocument->readBegin(FileName);

    //guidesdocument->readBegin("/home/rmr/kstars/kstars/data/skyguides.xml");


    guides = new GuidesListModel(guidesdocument->m_Guides);

    ctxt1 = qmlview->rootContext();
    ctxt1->setContextProperty("feedModel",guides);
    ctxt1->setContextProperty("slidesmodel",guides);

    qmlview->show();

}

