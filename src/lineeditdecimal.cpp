#include <QLocale>

#include "lineeditdecimal.h"

LineEditDecimal::LineEditDecimal(QWidget *parent) : QLineEdit(parent) {
  setAlignment(Qt::AlignRight);
  setProperty("value", 0.);
}

double LineEditDecimal::getValue() const { return QLocale(QLocale::Portuguese).toDouble(text()); }

void LineEditDecimal::setValue(const double value) { setText(QLocale(QLocale::Portuguese).toString(value, 'f', decimais)); }

void LineEditDecimal::setDecimais(const int value) { decimais = value; }
