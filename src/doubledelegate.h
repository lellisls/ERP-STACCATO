#ifndef DOUBLEDELEGATE_H
#define DOUBLEDELEGATE_H

#include <QStyledItemDelegate>

class DoubleDelegate : public QStyledItemDelegate {

  public:
    DoubleDelegate(QObject *parent = 0);
    ~DoubleDelegate();

    // QStyledItemDelegate interface
  public:
    QString displayText(const QVariant &value, const QLocale &locale) const;
};

#endif // DOUBLEDELEGATE_H
