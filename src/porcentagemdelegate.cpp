#include "porcentagemdelegate.h"

PorcentagemDelegate::PorcentagemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

PorcentagemDelegate::~PorcentagemDelegate() {}

QString PorcentagemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  if (value.userType() == QVariant::Double or value.userType() == QVariant::Int) {
    return locale.toString(value.toDouble() * 100., 'f', 1) + "%";
  }

  return QStyledItemDelegate::displayText(value, locale);
}
