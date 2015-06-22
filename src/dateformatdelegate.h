#ifndef DATEFORMATDELEGATE_H
#define DATEFORMATDELEGATE_H

#include <QStyledItemDelegate>
#include <QDate>

class DateFormatDelegate : public QStyledItemDelegate {

  public:
    DateFormatDelegate(QString dateFormat, QObject *parent = 0);
    ~DateFormatDelegate();
    virtual QString displayText(const QVariant &value, const QLocale &locale) const;

  private:
    const QString dateFormat;
};

#endif // DATEFORMATDELEGATE_H
