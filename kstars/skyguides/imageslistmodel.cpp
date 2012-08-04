#include "imageslistmodel.h"
#include "guidesdocument.h"
#include "guide.h"
#include "author.h"
#include "slide.h"
#include "image.h"

using namespace SkyGuidesSpace;

const int ImagesListModel::imagerole = Qt::UserRole + 1;

ImagesListModel::ImagesListModel(QList<Guide *> glist, QObject *parent) : QAbstractListModel(parent)
{
    guideslist= glist;

   // QHash roles = roleNames();

      roles.insert(imagerole, QByteArray("image"));

      setRoleNames(roles);

}

QVariant ImagesListModel::data(const QModelIndex &index, int role) const
{

        return QVariant();



}

int ImagesListModel::rowCount( const QModelIndex& parent ) const
{
//    if(currentIndex!=-1)
//    {
//        return guideslist[currentIndex]->m_Slides[]
//    }
    return 0;
}
