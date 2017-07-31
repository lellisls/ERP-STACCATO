#include <QDateTimeEdit>

#include "dateformatdelegate.h"

DateFormatDelegate::DateFormatDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QString DateFormatDelegate::displayText(const QVariant &value, const QLocale &) const { return value.toDate().toString("dd/MM/yyyy"); }

QWidget *DateFormatDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  auto *editor = new QDateTimeEdit(parent);
  editor->setDate(QDate::currentDate());
  editor->setDisplayFormat("dd/MM/yy");

  return editor;
}
