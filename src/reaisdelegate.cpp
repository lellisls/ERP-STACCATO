#include "reaisdelegate.h"

ReaisDelegate::ReaisDelegate(QObject *parent, const double decimais) : QStyledItemDelegate(parent), decimais(decimais) {}

QString ReaisDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  return value.userType() == QVariant::Double ? "R$ " + QLocale(QLocale::Portuguese).toString(value.toDouble(), 'f', decimais) : QStyledItemDelegate::displayText(value, locale);
}

// TODO: add a parameter for enabling/disabling editing (also in others delegate so as to remove NoEditDelegate)
