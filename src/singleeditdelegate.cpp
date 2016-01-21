#include "singleeditdelegate.h"

SingleEditDelegate::SingleEditDelegate(int column, QObject *parent) : QStyledItemDelegate(parent), column(column) {}

QWidget *SingleEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const {
  if (index.column() != column) return 0;

  return QStyledItemDelegate::createEditor(parent, option, index);
}
