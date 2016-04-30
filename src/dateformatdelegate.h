#ifndef DATEFORMATDELEGATE_H
#define DATEFORMATDELEGATE_H

#include <QDate>
#include <QStyledItemDelegate>

class DateFormatDelegate : public QStyledItemDelegate {

public:
  explicit DateFormatDelegate(QObject *parent = 0);
  ~DateFormatDelegate();

private:
  virtual QString displayText(const QVariant &value, const QLocale &) const override;
};

#endif // DATEFORMATDELEGATE_H
