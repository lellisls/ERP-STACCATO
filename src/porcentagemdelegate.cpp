#include "porcentagemdelegate.h"

PorcentagemDelegate::PorcentagemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

PorcentagemDelegate::~PorcentagemDelegate() {}

QString PorcentagemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  Q_UNUSED(locale);

  const QLocale local;

  if (value.userType() == QVariant::Double or value.userType() == QVariant::Int) {
    return local.toString(value.toDouble() * 100, 'f', 0) + "%";
  } else {
    return QStyledItemDelegate::displayText(value, local);
  }
}
