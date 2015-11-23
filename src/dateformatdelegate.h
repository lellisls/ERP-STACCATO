#ifndef DATEFORMATDELEGATE_H
#define DATEFORMATDELEGATE_H

#include <QStyledItemDelegate>
#include <QDate>

class DateFormatDelegate : public QStyledItemDelegate {

  public:
    explicit DateFormatDelegate(QObject *parent = 0);
    ~DateFormatDelegate();

  private:
    // attributes
    // methods
    virtual QString displayText(const QVariant &value, const QLocale &) const;
};

#endif // DATEFORMATDELEGATE_H
