#include "doubledelegate.h"

DoubleDelegate::DoubleDelegate(QObject *parent, const double &decimais) : QStyledItemDelegate(parent), decimais(decimais) {}

DoubleDelegate::~DoubleDelegate() {}

QString DoubleDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  return value.userType() == QVariant::Double ? locale.toString(value.toDouble(), 'f', decimais)
                                              : QStyledItemDelegate::displayText(value, locale);
}
