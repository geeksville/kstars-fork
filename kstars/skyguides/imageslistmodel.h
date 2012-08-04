#include <QAbstractListModel>
#include "skyguidesspace.h"

class SkyGuidesSpace::ImagesListModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit ImagesListModel(QList<Guide*>glist ,QObject* parent=0);
    int rowCount( const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role=0) const;
    QHash<int, QByteArray > roles;
    int currentIndex;

private:
    QList<Guide*> guideslist;
    static const int imagerole;
};
