#ifndef EDITABLESQLMODEL_H
#define EDITABLESQLMODEL_H

#include <QSqlRelationalTableModel>

class EditableSqlModel : public QSqlRelationalTableModel {
    Q_OBJECT

  public:
    EditableSqlModel(QObject *parent = 0);
    ~EditableSqlModel();

    // QAbstractItemModel interface
    Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif // EDITABLESQLMODEL_H
