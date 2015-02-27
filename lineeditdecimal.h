#ifndef LINEEDITDECIMAL_H
#define LINEEDITDECIMAL_H

#include <QLineEdit>
#include <QObject>

class LineEditDecimal : public QLineEdit {
  Q_OBJECT

public:
  explicit LineEditDecimal(QWidget *parent = 0);
  double getBottom() const;
  void setBottom(double value);
  double getTop() const;
  void setTop(double value);
  double getValue() const;
  void setValue(double value);

private slots:
  void processDecimal(QString value);

private:
  Q_PROPERTY(double value READ getValue WRITE setValue STORED false)
  //attributes
  double bottom;
  double top;

};

#endif // LINEEDITDECIMAL_H
