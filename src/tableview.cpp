#include <QDebug>
#include <QHeaderView>
#include <QIdentityProxyModel>
#include <QSqlRecord>
#include <QSqlTableModel>

#include "tableview.h"

TableView::TableView(QWidget *parent) : QTableView(parent) {
  verticalHeader()->setResizeContentsPrecision(0);
  horizontalHeader()->setResizeContentsPrecision(0);

  verticalHeader()->setDefaultSectionSize(20);
}

void TableView::hideColumn(const QString &column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::hideColumn(sourceModel->fieldIndex(column));
      return;
    }

    if (auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::hideColumn(sourceModel->record().indexOf(column));
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::hideColumn(model->fieldIndex(column));
    return;
  }

  if (auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::hideColumn(model->record().indexOf(column));
    return;
  }
}

void TableView::showColumn(const QString &column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::showColumn(sourceModel->fieldIndex(column));
      return;
    }

    if (auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::showColumn(sourceModel->record().indexOf(column));
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::showColumn(model->fieldIndex(column));
    return;
  }

  if (auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::showColumn(model->record().indexOf(column));
    return;
  }
}

void TableView::setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::setItemDelegateForColumn(sourceModel->fieldIndex(column), delegate);
      return;
    }

    if (auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::setItemDelegateForColumn(sourceModel->record().indexOf(column), delegate);
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::setItemDelegateForColumn(model->fieldIndex(column), delegate);
    return;
  }

  if (auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::setItemDelegateForColumn(model->record().indexOf(column), delegate);
    return;
  }
}

void TableView::openPersistentEditor(const int row, const QString &column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::openPersistentEditor(model->index(row, sourceModel->fieldIndex(column)));
      return;
    }

    if (auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::openPersistentEditor(model->index(row, sourceModel->record().indexOf(column)));
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, model->fieldIndex(column)));
    return;
  }

  if (auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, model->record().indexOf(column)));
    return;
  }
}

void TableView::openPersistentEditor(const int row, const int column) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, column));
    return;
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, column));
    return;
  }

  if (auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, column));
    return;
  }
}

void TableView::setItemDelegateForColumn(const int column, QAbstractItemDelegate *delegate) { QTableView::setItemDelegateForColumn(column, delegate); }

void TableView::sortByColumn(const QString &column, Qt::SortOrder order) {
  if (auto *model = qobject_cast<QIdentityProxyModel *>(QTableView::model())) {
    if (auto *sourceModel = qobject_cast<QSqlTableModel *>(model->sourceModel())) {
      QTableView::sortByColumn(sourceModel->fieldIndex(column), order);
      return;
    }

    if (auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::sortByColumn(sourceModel->record().indexOf(column), order);
      return;
    }
  }

  if (auto *model = qobject_cast<QSqlTableModel *>(QTableView::model())) {
    QTableView::sortByColumn(model->fieldIndex(column), order);
    return;
  }

  if (auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::sortByColumn(model->record().indexOf(column), order);
    return;
  }
}

void TableView::setModel(QAbstractItemModel *model) {
  QTableView::setModel(model);

  hideColumn("created");
  hideColumn("lastUpdated");
}

// TODO: program copy - http://stackoverflow.com/questions/3135737/copying-part-of-qtableview
