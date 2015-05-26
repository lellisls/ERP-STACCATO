#include "dateformatdelegate.h"

DateFormatDelegate::DateFormatDelegate(QString dateFormat, QObject *parent) : QStyledItemDelegate(parent), dateFormat(dateFormat) {}

DateFormatDelegate::~DateFormatDelegate() {}

QString DateFormatDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  Q_UNUSED(locale);

  return value.toDate().toString(dateFormat);
}
