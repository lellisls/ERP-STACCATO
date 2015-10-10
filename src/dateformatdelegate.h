#ifndef DATEFORMATDELEGATE_H
#define DATEFORMATDELEGATE_H

#include <QStyledItemDelegate>
#include <QDate>

class DateFormatDelegate : public QStyledItemDelegate {

  public:
    DateFormatDelegate(QString dateFormat, QObject *parent = 0);
    ~DateFormatDelegate();

  private:
    // attributes
    const QString dateFormat;
    // methods
    virtual QString displayText(const QVariant &value, const QLocale &locale) const;
};

#endif // DATEFORMATDELEGATE_H
