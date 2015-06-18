#ifndef PORCENTAGEMDELEGATE_H
#define PORCENTAGEMDELEGATE_H

#include <QStyledItemDelegate>

class PorcentagemDelegate : public QStyledItemDelegate {
  public:
    explicit PorcentagemDelegate(QObject *parent = 0);
    ~PorcentagemDelegate();

    // QStyledItemDelegate interface
  public:
    QString displayText(const QVariant &value, const QLocale &locale) const;
};

#endif // PORCENTAGEMDELEGATE_H
