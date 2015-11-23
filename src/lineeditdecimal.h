#ifndef LINEEDITDECIMAL_H
#define LINEEDITDECIMAL_H

#include <QLineEdit>

class LineEditDecimal : public QLineEdit {
    Q_OBJECT

  public:
    explicit LineEditDecimal(QWidget *parent = 0);
    double getValue() const;
    void setValue(const double &value);

  private:
    Q_PROPERTY(double value READ getValue WRITE setValue STORED false)
    // attributes
    double bottom;
    double top;
    // methods
    double getBottom() const;
    void setBottom(const double &value);
    double getTop() const;
    void setTop(const double &value);
};

#endif // LINEEDITDECIMAL_H
