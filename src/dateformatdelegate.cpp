#include "dateformatdelegate.h"

DateFormatDelegate::DateFormatDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

DateFormatDelegate::~DateFormatDelegate() {}

QString DateFormatDelegate::displayText(const QVariant &value, const QLocale &) const {
  return value.toDate().toString("dd-MM-yyyy");
}
