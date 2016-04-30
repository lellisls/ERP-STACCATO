#ifndef SINGLEEDITDELEGATE_H
#define SINGLEEDITDELEGATE_H

#include <QStyledItemDelegate>

class SingleEditDelegate : public QStyledItemDelegate {

public:
  SingleEditDelegate(int column, QObject *parent = 0);
  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
  int column;
};

#endif // SINGLEEDITDELEGATE_H
