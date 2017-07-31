#ifndef DOUBLEDELEGATE_H
#define DOUBLEDELEGATE_H

#include <QStyledItemDelegate>

class DoubleDelegate : public QStyledItemDelegate {

public:
  explicit DoubleDelegate(QObject *parent = 0, const int decimais = 2);
  ~DoubleDelegate() = default;
  QString displayText(const QVariant &value, const QLocale &locale) const override;

private:
  int decimais;
};

#endif // DOUBLEDELEGATE_H
