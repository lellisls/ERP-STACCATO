#include "reaisdelegate.h"

ReaisDelegate::ReaisDelegate(QObject *parent, const double decimais)
    : QStyledItemDelegate(parent), decimais(decimais) {}

ReaisDelegate::~ReaisDelegate() {}

QString ReaisDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  return value.userType() == QVariant::Double
             ? "R$ " + QLocale(QLocale::Portuguese).toString(value.toDouble(), 'f', decimais)
             : QStyledItemDelegate::displayText(value, locale);
}
