#include "noeditdelegate.h"

NoEditDelegate::NoEditDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QWidget *NoEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
  Q_UNUSED(parent);
  Q_UNUSED(option);
  Q_UNUSED(index);

  return 0;
}
