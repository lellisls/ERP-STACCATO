#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class TableView : public QTableView {

public:
  explicit TableView(QWidget *parent = 0);
  virtual void setModel(QAbstractItemModel *model) override;
  void showColumn(const QString &column);
  void hideColumn(const QString &column);
  void openPersistentEditor(const int row, const int column);
  void openPersistentEditor(const int row, const QString &column);
  void setItemDelegateForColumn(const int column, QAbstractItemDelegate *delegate);
  void setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate);

public slots:
  void sortByColumn(const QString &column, Qt::SortOrder order = Qt::AscendingOrder);
};

#endif // TABLEVIEW_H
