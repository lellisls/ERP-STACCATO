#ifndef NOEDITDELEGATE_H
#define NOEDITDELEGATE_H

#include <QStyledItemDelegate>

class NoEditDelegate : public QStyledItemDelegate {

public:
  explicit NoEditDelegate(QObject *parent = 0);
  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &) const;
};

#endif // NOEDITDELEGATE_H
