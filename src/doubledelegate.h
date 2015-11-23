#ifndef DOUBLEDELEGATE_H
#define DOUBLEDELEGATE_H

#include <QStyledItemDelegate>

class DoubleDelegate : public QStyledItemDelegate {

  public:
    explicit DoubleDelegate(QObject *parent = 0, const double &decimais = 2.);
    ~DoubleDelegate();
    // QStyledItemDelegate interface
    QString displayText(const QVariant &value, const QLocale &locale) const;

  private:
    double decimais;
};

#endif // DOUBLEDELEGATE_H
