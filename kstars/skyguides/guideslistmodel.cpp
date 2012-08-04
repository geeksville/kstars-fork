#include "guideslistmodel.h"
#include "guidesdocument.h"
#include "guide.h"
#include "author.h"
#include "slide.h"
#include "image.h"

using namespace SkyGuidesSpace;

const int GuidesListModel::titlerole = Qt::UserRole + 1;
const int GuidesListModel::descriptionrole = Qt::UserRole + 2;
const int GuidesListModel::thumbimagerole = Qt::UserRole + 3;

GuidesListModel::GuidesListModel(QList<Guide *> glist, QObject *parent) : QAbstractListModel(parent)
{
    guideslist= glist;

      roles.insert(titlerole, QByteArray("title"));
      roles.insert(descriptionrole, QByteArray("description"));
      roles.insert(thumbimagerole, QByteArray("thumbimage"));

      setRoleNames(roles);

}

QVariant GuidesListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > rowCount())
        return QVariant();

    if(role==titlerole)
    {
        return guideslist[index.row()]->m_Slides[0]->m_Images[0]->m_Url;
    }
    else if(role==thumbimagerole)
    {
        return guideslist[index.row()]->thumbnailImage();

    }
    else
        return QVariant();



}

int GuidesListModel::rowCount( const QModelIndex& parent ) const
{
    return guideslist.size();
}
