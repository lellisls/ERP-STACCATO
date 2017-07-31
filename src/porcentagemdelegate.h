#ifndef PORCENTAGEMDELEGATE_H
#define PORCENTAGEMDELEGATE_H

#include <QStyledItemDelegate>

class PorcentagemDelegate : public QStyledItemDelegate {

public:
  explicit PorcentagemDelegate(QObject *parent = 0);
  ~PorcentagemDelegate() = default;
  QString displayText(const QVariant &value, const QLocale &locale) const override;
};

#endif // PORCENTAGEMDELEGATE_H
