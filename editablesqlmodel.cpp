#include "editablesqlmodel.h"

EditableSqlModel::EditableSqlModel(QObject *parent) : QSqlRelationalTableModel(parent)
{

}

EditableSqlModel::~EditableSqlModel()
{

}

Qt::ItemFlags EditableSqlModel::flags(const QModelIndex &index) const{
    Qt::ItemFlags flags = QSqlQueryModel::flags(index);
    flags |= Qt::ItemIsEditable;
    return flags;
}
