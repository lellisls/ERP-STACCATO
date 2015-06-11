#include "doubledelegate.h"

DoubleDelegate::DoubleDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

DoubleDelegate::~DoubleDelegate() {}

QString DoubleDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  Q_UNUSED(locale);
  QLocale local;

  if (value.userType() == QVariant::Double) {
    return local.toString(value.toDouble(), 'f', 2);
  } else {
    return QStyledItemDelegate::displayText(value, local);
  }
}
