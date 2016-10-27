#include <QHeaderView>
#include <QIdentityProxyModel>
#include <QSqlTableModel>

#include "tableview.h"

TableView::TableView(QWidget *parent) : QTableView(parent) {
  verticalHeader()->setResizeContentsPrecision(0);
  horizontalHeader()->setResizeContentsPrecision(0);
}

void TableView::hideColumn(const QString &column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::hideColumn(sourceModel->fieldIndex(column));
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::hideColumn(model->fieldIndex(column));
  }
}

void TableView::setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::setItemDelegateForColumn(sourceModel->fieldIndex(column), delegate);
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::setItemDelegateForColumn(model->fieldIndex(column), delegate);
  }
}

void TableView::openPersistentEditor(const int &row, const QString &column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::openPersistentEditor(model->index(row, sourceModel->fieldIndex(column)));
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, model->fieldIndex(column)));
  }
}

void TableView::openPersistentEditor(const int &row, const int &column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, column));
    return;
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, column));
  }
}

void TableView::setItemDelegateForColumn(const int &column, QAbstractItemDelegate *delegate) {
  QTableView::setItemDelegateForColumn(column, delegate);
}

void TableView::sortByColumn(const QString &column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::sortByColumn(sourceModel->fieldIndex(column));
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::sortByColumn(model->fieldIndex(column));
  }
}

void TableView::setModel(QAbstractItemModel *model) {
  QTableView::setModel(model);

  hideColumn("created");
  hideColumn("lastUpdated");
}
