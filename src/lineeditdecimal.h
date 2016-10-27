#ifndef LINEEDITDECIMAL_H
#define LINEEDITDECIMAL_H

#include <QLineEdit>

class LineEditDecimal : public QLineEdit {
  Q_OBJECT

public:
  explicit LineEditDecimal(QWidget *parent = 0);
  double getValue() const;
  void setValue(const double value);
  void setDecimais(const int value);

private:
  Q_PROPERTY(double value READ getValue WRITE setValue STORED false)
  // attributes
  int decimais = 2;
  // methods
};

#endif // LINEEDITDECIMAL_H
