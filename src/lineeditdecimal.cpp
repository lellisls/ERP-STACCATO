#include <QLocale>

#include "lineeditdecimal.h"

LineEditDecimal::LineEditDecimal(QWidget *parent) : QLineEdit(parent) {
  setAlignment(Qt::AlignRight);
  setProperty("value", 0.);
}

double LineEditDecimal::getValue() const { return QLocale(QLocale::Portuguese).toDouble(text()); }

void LineEditDecimal::setValue(const double &value) {
  setText(QLocale(QLocale::Portuguese).toString(value, 'f', decimais));
}

void LineEditDecimal::setDecimais(double value) { decimais = value; }

double LineEditDecimal::getBottom() const { return bottom; }

void LineEditDecimal::setBottom(const double &value) { bottom = value; }

double LineEditDecimal::getTop() const { return top; }

void LineEditDecimal::setTop(const double &value) { top = value; }
