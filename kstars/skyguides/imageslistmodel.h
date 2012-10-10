#include <QAbstractListModel>
#include "skyguidesspace.h"

class SkyGuidesSpace::ImagesListModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit ImagesListModel(Slide* s ,QObject* parent=0);
    int rowCount( const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role=0) const;
    QHash<int, QByteArray > roles;
    int currentIndex;

private:
    Slide* slide;
    static const int imagerole;
};
