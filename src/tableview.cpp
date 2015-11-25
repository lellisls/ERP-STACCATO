#include <QHeaderView>
#include <QIdentityProxyModel>
#include <QSqlTableModel>

#include "tableview.h"

TableView::TableView(QWidget *parent) : QTableView(parent) {
  verticalHeader()->setResizeContentsPrecision(0);
  horizontalHeader()->setResizeContentsPrecision(0);
}

void TableView::hideColumn(const QString &column) {
  if (QIdentityProxyModel *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (QSqlTableModel *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::hideColumn(sourceModel->fieldIndex(column));
    }
  }

  if (QSqlTableModel *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::hideColumn(model->fieldIndex(column));
  }
}

void TableView::setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate) {
  if (QIdentityProxyModel *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (QSqlTableModel *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::setItemDelegateForColumn(sourceModel->fieldIndex(column), delegate);
    }
  }

  if (QSqlTableModel *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::setItemDelegateForColumn(model->fieldIndex(column), delegate);
  }
}

void TableView::setItemDelegateForColumn(const int &column, QAbstractItemDelegate *delegate) {
  QTableView::setItemDelegateForColumn(column, delegate);
}

void TableView::sortByColumn(const QString &column) {
  if (QIdentityProxyModel *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (QSqlTableModel *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::sortByColumn(sourceModel->fieldIndex(column));
    }
  }

  if (QSqlTableModel *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::sortByColumn(model->fieldIndex(column));
  }
}
