#ifndef REAISDELEGATE_H
#define REAISDELEGATE_H

#include <QStyledItemDelegate>

class ReaisDelegate : public QStyledItemDelegate {

public:
  explicit ReaisDelegate(QObject *parent = 0, const double decimais = 2.);
  ~ReaisDelegate() = default;
  QString displayText(const QVariant &value, const QLocale &locale) const override;

private:
  const double decimais;
};

#endif // REAISDELEGATE_H
