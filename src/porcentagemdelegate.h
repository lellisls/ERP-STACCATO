#ifndef PORCENTAGEMDELEGATE_H
#define PORCENTAGEMDELEGATE_H

#include <QStyledItemDelegate>

class PorcentagemDelegate : public QStyledItemDelegate {
  public:
    explicit PorcentagemDelegate(bool division = false, QObject *parent = 0);
    ~PorcentagemDelegate();
    // QStyledItemDelegate interface
    QString displayText(const QVariant &value, const QLocale &locale) const;

  private:
    bool division;
};

#endif // PORCENTAGEMDELEGATE_H
