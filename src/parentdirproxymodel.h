#ifndef PARENTDIRPROXYMODEL_H
#define PARENTDIRPROXYMODEL_H

#include <QSortFilterProxyModel>

class ParentDirProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ParentDirProxyModel(QObject *parent = nullptr);
};

#endif // PARENTDIRPROXYMODEL_H
