#include "porcentagemdelegate.h"

PorcentagemDelegate::PorcentagemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QString PorcentagemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  if (value.userType() == QVariant::Double or value.userType() == QVariant::Int) return locale.toString(value.toDouble(), 'f', 2) + " %";

  return QStyledItemDelegate::displayText(value, locale);
}
