#ifndef DOUBLEDELEGATE_H
#define DOUBLEDELEGATE_H

#include <QStyledItemDelegate>

class DoubleDelegate : public QStyledItemDelegate {

  public:
    explicit DoubleDelegate(QObject *parent = 0);
    explicit DoubleDelegate(double decimais, QObject *parent = 0);
    ~DoubleDelegate();
    // QStyledItemDelegate interface
    QString displayText(const QVariant &value, const QLocale &locale) const;

  private:
    double decimais = 2;
};

#endif // DOUBLEDELEGATE_H
