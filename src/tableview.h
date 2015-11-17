#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class TableView : public QTableView {
  public:
    explicit TableView(QWidget *parent = 0);
    void hideColumn(QString column);
    void setItemDelegateForColumn(QString column, QAbstractItemDelegate *delegate);
    void setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate);

  public slots:
    void sortByColumn(QString column);
};

#endif // TABLEVIEW_H
