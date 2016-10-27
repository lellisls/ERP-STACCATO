#ifndef SINGLEEDITDELEGATE_H
#define SINGLEEDITDELEGATE_H

#include <QStyledItemDelegate>

class SingleEditDelegate : public QStyledItemDelegate {

public:
  explicit SingleEditDelegate(QObject *parent = 0);
  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  virtual QString displayText(const QVariant &value, const QLocale &locale) const override;
};

#endif // SINGLEEDITDELEGATE_H
