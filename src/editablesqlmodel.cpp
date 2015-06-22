#include "editablesqlmodel.h"

EditableSqlModel::EditableSqlModel(QObject *parent) : QSqlRelationalTableModel(parent) {}

EditableSqlModel::~EditableSqlModel() {}

Qt::ItemFlags EditableSqlModel::flags(const QModelIndex &index) const {
  const Qt::ItemFlags flags = QSqlQueryModel::flags(index) | Qt::ItemIsEditable;

  return flags;
}
