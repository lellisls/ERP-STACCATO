#include "doubledelegate.h"

DoubleDelegate::DoubleDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

DoubleDelegate::DoubleDelegate(double decimais, QObject *parent) : QStyledItemDelegate(parent), decimais(decimais) {}

DoubleDelegate::~DoubleDelegate() {}

QString DoubleDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  Q_UNUSED(locale);

  const QLocale local;

  return value.userType() == QVariant::Double ? local.toString(value.toDouble(), 'f', decimais)
                                              : QStyledItemDelegate::displayText(value, local);
}
