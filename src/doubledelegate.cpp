#include "doubledelegate.h"

DoubleDelegate::DoubleDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

DoubleDelegate::~DoubleDelegate() {}

QString DoubleDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  if (value.userType() == QVariant::Double) {
    return locale.toString(value.toDouble(), 'f', 2);
  } else {
    return QStyledItemDelegate::displayText(value, locale);
  }
}
