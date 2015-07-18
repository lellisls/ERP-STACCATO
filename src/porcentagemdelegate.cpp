#include "porcentagemdelegate.h"

PorcentagemDelegate::PorcentagemDelegate(bool division, QObject *parent)
  : QStyledItemDelegate(parent), division(division) {}

PorcentagemDelegate::~PorcentagemDelegate() {}

QString PorcentagemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  Q_UNUSED(locale);

  const QLocale local;

  if (value.userType() == QVariant::Double or value.userType() == QVariant::Int) {
    if (division) {
      return local.toString(value.toDouble() * 100, 'f', 0) + "%";
    } else {
      return local.toString(value.toDouble(), 'f', 0) + "%";
    }
  } else {
    return QStyledItemDelegate::displayText(value, local);
  }
}
