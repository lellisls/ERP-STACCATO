#include "singleeditdelegate.h"

SingleEditDelegate::SingleEditDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QWidget *SingleEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const { return QStyledItemDelegate::createEditor(parent, option, index); }

QString SingleEditDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  return value.userType() == QVariant::Double ? QLocale(QLocale::Portuguese).toString(value.toDouble(), 'f', 3) : QStyledItemDelegate::displayText(value, locale);
}
