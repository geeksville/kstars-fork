#include "slideslistmodel.h"
#include "guidesdocument.h"
#include "guide.h"
#include "author.h"
#include "slide.h"
#include "image.h"

using namespace SkyGuidesSpace;

const int SlidesListModel::titlerole = Qt::UserRole + 1;


SlidesListModel::SlidesListModel(QList<Guide *> glist, QObject *parent) : QAbstractListModel(parent)
{
    guideslist= glist;

    // QHash roles = roleNames();
    roles.insert(titlerole, QByteArray("title"));

    setRoleNames(roles);

}

QVariant SlidesListModel::data(const QModelIndex &index, int role) const
{

    if (index.row() < 0 || index.row() > rowCount())
        return QVariant();

    if(currentIndex>=0)
    {
        if(role==titlerole)
            return guideslist[currentIndex]->m_Slides[index.row()]->title();
        else
            return QVariant();
    }
    else
        return QVariant();
}

int SlidesListModel::rowCount( const QModelIndex& parent ) const
{
    return guideslist[currentIndex]->m_Slides.size();
}
