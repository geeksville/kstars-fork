#include "onlineimagebrowser.h"

#include "services/astrobin/astrobinapi.h"
#include "services/astrobin/astrobinapijson.h"
#include "services/gimagessearch.h"

#include "skyobject.h"

#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <KJob>
#include <KMessageBox>
#include <KFileDialog>

#include <QDesktopWidget>
#include <QNetworkAccessManager>
#include <QListWidgetItem>


class ImageListItem : public QListWidgetItem
{
    Q_OBJECT

public:
    ImageListItem(const QIcon &icon, const QString &text, QPixmap *pixmap, QListWidget *parent = 0, int type = Type)
        : QListWidgetItem(icon, text, parent, type), m_Pixmap(pixmap)
    {}

    QPixmap* pixmap() { return m_Pixmap; }

private:
    QPixmap *m_Pixmap;
};



OnlineImageBrowser_ui::OnlineImageBrowser_ui(QWidget *parent)
    : QFrame(parent)
{
    setupUi(this);
}

OnlineImageBrowser::OnlineImageBrowser(SkyObject *obj, bool thumbnailPickMode, QWidget *parent)
    : KDialog(parent), m_ThumbnailPickMode(thumbnailPickMode), m_Object(obj), m_SearchType(ASTROBIN)
{
    m_Ui = new OnlineImageBrowser_ui(this);
    setMainWidget(m_Ui);

    if(!m_ThumbnailPickMode) {
        m_Ui->editButton->hide();
        m_Ui->unsetButton->hide();
    }

    setCaption(i18n("Online Image Browser"));
    m_Ui->splitter->setStretchFactor(0, 1);
    m_Ui->splitter->setStretchFactor(1, 5);

    m_NetworkManager = new QNetworkAccessManager(this);

    // Create AstroBinApi
    m_AstrobinApi = new AstroBinApiJson(m_NetworkManager, this);

    // Create Google Images Api
    m_GImagesApi = new GImagesSearch(m_NetworkManager, this);

    connect(m_Ui->astrobinRadioButton, SIGNAL(clicked()), this, SLOT(slotAstrobinSearch()));
    connect(m_Ui->googleRadioButton, SIGNAL(clicked()), this, SLOT(slotGoogleSearch()));

    connect(m_AstrobinApi, SIGNAL(searchFinished(bool)), this, SLOT(slotAstrobinSearchCompleted(bool)));
    connect(m_GImagesApi, SIGNAL(searchFinished(bool)), this, SLOT(slotGoogleSearchCompleted(bool)));

    connect(m_Ui->imageListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(slotSetFromList(int)));

    connect(m_Ui->unsetButton, SIGNAL(clicked()), this, SLOT(slotUnset()));
    connect(m_Ui->saveButton, SIGNAL(clicked()), this, SLOT(slotSave()));
    connect(m_Ui->editButton, SIGNAL(clicked()), this, SLOT(slotEdit()));

    slotAstrobinSearch();
}

OnlineImageBrowser::~OnlineImageBrowser()
{
    deleteImages();
    qDeleteAll(m_Jobs);
}

void OnlineImageBrowser::slotAstrobinSearch()
{
    clearImagesList();
    m_SearchType = ASTROBIN;
    m_AstrobinApi->searchObjectImages(m_Object->name());
}

void OnlineImageBrowser::slotAstrobinSearchCompleted(bool ok)
{
    if(!ok) {
        KMessageBox::error(this, i18n("AstroBin.com search error: ", i18n("AstroBin.com search error")));
        return;
    }

    AstroBinSearchResult result = m_AstrobinApi->getResult();
    foreach(AstroBinImage image, result) {
        KIO::StoredTransferJob *job = KIO::storedGet(image.downloadResizedUrl(), KIO::NoReload, KIO::HideProgressInfo);
        job->setUiDelegate(0);
        m_Jobs.append(job);
        connect(job, SIGNAL(result(KJob*)), SLOT(slotJobResult(KJob*)));
    }
}

void OnlineImageBrowser::slotGoogleSearch()
{
    clearImagesList();
    m_SearchType = GOOGLE_IMAGES;
    m_GImagesApi->searchObjectImages(m_Object);
}

void OnlineImageBrowser::slotGoogleSearchCompleted(bool ok)
{
    if(!ok) {
        KMessageBox::error(this, i18n("Google Images search error: ", i18n("Google Images search error")));
        return;
    }

    foreach(QUrl url, m_GImagesApi->getResult()) {
        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
        job->setUiDelegate(0);
        m_Jobs.append(job);
        connect(job, SIGNAL(result(KJob*)), SLOT(slotJobResult(KJob*)));
    }
}

void OnlineImageBrowser::slotSetFromList(int row)
{
    if(row < 0 || row >= m_Images.size()) {
        return;
    }

    m_Ui->imageLabel->setPixmap(*m_Images.at(row).first);
}

void OnlineImageBrowser::slotJobResult(KJob *job)
{
    KIO::StoredTransferJob *storedTranferJob = (KIO::StoredTransferJob*)job;
    m_Jobs.removeOne(storedTranferJob);

    //If there was a problem, just return silently without adding image to list.
    if(job->error()) {
        job->kill();
        return;
    }

    QPixmap *pm = new QPixmap();
    pm->loadFromData(storedTranferJob->data());

    uint w = pm->width();
    uint h = pm->height();
    uint pad = 0;
    uint hDesk = QApplication::desktop()->availableGeometry().height() - pad;

    if(h > hDesk)
        *pm = pm->scaled(w * hDesk / h, hDesk, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_Images.append(QPair<QPixmap*, DATA_SOURCE>(pm, m_SearchType));

    //Add 50x50 image and URL to listbox
    m_Ui->imageListWidget->addItem(new QListWidgetItem(QIcon(*pm), QString()));
}

void OnlineImageBrowser::slotUnset()
{

}

void OnlineImageBrowser::slotSave()
{
    int currentRow = m_Ui->imageListWidget->currentRow();
    if(currentRow < 0 || currentRow >= m_Images.size()) {
        return;
    }

    KUrl fileUrl = KFileDialog::getSaveUrl(QDir::homePath(),
                                           "image/png image/jpeg image/gif image/x-portable-pixmap image/bmp",
                                           this);
    if(!fileUrl.isValid()) {
        return;
    }

    QString fileName(fileUrl.toLocalFile());
    QFile file(fileName);
    file.open(QFile::WriteOnly);

    if(fileUrl.isValid()) {
        QString extension = fileName.mid(fileName.lastIndexOf(".") + 1);
        m_Images.at(currentRow).first->save(&file, extension.toLocal8Bit());
    }
}

void OnlineImageBrowser::slotEdit()
{

}

void OnlineImageBrowser::clearImagesList()
{
    // Kill all running jobs
    killAllRunningJobs();


    m_Ui->imageLabel->clear();
    m_Ui->imageListWidget->clear();
}

void OnlineImageBrowser::killAllRunningJobs()
{
    foreach(KIO::StoredTransferJob *job, m_Jobs) {
        job->kill();
    }

    m_Jobs.clear();
}

void OnlineImageBrowser::deleteImages()
{
    QPair<QPixmap*, DATA_SOURCE> image;
    foreach(image, m_Images) {
        delete image.first;
    }
}


