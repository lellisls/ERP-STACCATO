#include "tableview.h"

#include <QHeaderView>
#include <QSqlTableModel>

TableView::TableView(QWidget *parent) : QTableView(parent) {
  verticalHeader()->setResizeContentsPrecision(0);
  horizontalHeader()->setResizeContentsPrecision(0);
}

void TableView::hideColumn(const QString column) {
  if (QSqlTableModel *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::hideColumn(model->fieldIndex(column));
  }
}

void TableView::setItemDelegateForColumn(const QString column, QAbstractItemDelegate *delegate) {
  if (QSqlTableModel *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::setItemDelegateForColumn(model->fieldIndex(column), delegate);
  }
}

void TableView::setItemDelegateForColumn(const int &column, QAbstractItemDelegate *delegate) {
  QTableView::setItemDelegateForColumn(column, delegate);
}

void TableView::sortByColumn(const QString &column) {
  if (QSqlTableModel *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::sortByColumn(model->fieldIndex(column));
  }
}
