#include "imageslistmodel.h"
#include "guidesdocument.h"
#include "guide.h"
#include "author.h"
#include "slide.h"
#include "image.h"

using namespace SkyGuidesSpace;

const int ImagesListModel::imagerole = Qt::UserRole + 1;

ImagesListModel::ImagesListModel(Slide * g, QObject *parent) : QAbstractListModel(parent)
{
    slide = g;

      roles.insert(imagerole, QByteArray("image"));

      setRoleNames(roles);

}

QVariant ImagesListModel::data(const QModelIndex &index, int role) const
{

    if (index.row() < 0 || index.row() > rowCount())
        return QVariant();

    if(role==imagerole)
    {
        return slide->m_Images[index.row()]->m_Url;
    }
    else
        return QVariant();

}

int ImagesListModel::rowCount( const QModelIndex& parent ) const
{
    return slide->m_Images.size();

//    if(currentIndex!=-1)
//    {
//        return guideslist[currentIndex]->m_Slides[]
//    }
//    return 0;
}
