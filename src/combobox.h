#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>

class ComboBox : public QComboBox {
  Q_OBJECT

public:
  explicit ComboBox(QWidget *parent);
  ~ComboBox() = default;
  QVariant getCurrentValue() const;

public slots:
  bool setCurrentValue(const QVariant &value);

private:
  Q_PROPERTY(QVariant currentValue READ getCurrentValue WRITE setCurrentValue STORED false)
};

#endif // COMBOBOX_H
