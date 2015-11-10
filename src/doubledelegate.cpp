#include "doubledelegate.h"

DoubleDelegate::DoubleDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

DoubleDelegate::DoubleDelegate(double decimais, QObject *parent) : QStyledItemDelegate(parent), decimais(decimais) {}

DoubleDelegate::~DoubleDelegate() {}

QString DoubleDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  return value.userType() == QVariant::Double ? locale.toString(value.toDouble(), 'f', decimais)
                                              : QStyledItemDelegate::displayText(value, locale);
}
