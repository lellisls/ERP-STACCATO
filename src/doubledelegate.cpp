#include "doubledelegate.h"

DoubleDelegate::DoubleDelegate(QObject *parent, const int decimais) : QStyledItemDelegate(parent), decimais(decimais) {}

QString DoubleDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  return value.userType() == QVariant::Double ? QLocale(QLocale::Portuguese).toString(value.toDouble(), 'f', decimais) : QStyledItemDelegate::displayText(value, locale);
}
